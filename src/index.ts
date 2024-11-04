import { BrowserWindow, ipcMain, screen, shell } from "electron";
import { loadNativeLib } from "./main/utils/loadoverlay";
import { fileUrl } from "./main/utils/utils";
import * as path from "path";

enum AppWindows {
    main = "main",
    osr = "osr",
    osrpopup = "osrpopup",
}

class IngameOverlay {
    private windows: Map<string, Electron.BrowserWindow>;
    private markQuit = false;

    private Overlay;
    private scaleFactor = 1.0;

    constructor() {
        this.windows = new Map();
        this.Overlay = loadNativeLib();
    }

    get mainWindow() {
        return this.windows.get(AppWindows.main) || null;
    }

    set mainWindow(window: Electron.BrowserWindow | null) {
        if (!window) {
            this.windows.delete(AppWindows.main);
        } else {
            this.windows.set(AppWindows.main, window);
            window.on("closed", () => {
                this.mainWindow = null;
            });

            window.loadURL(global.CONFIG.entryUrl);

            window.on("ready-to-show", () => {
                this.showAndFocusWindow(AppWindows.main);
            });

            window.webContents.on("did-fail-load", () => {
                window.reload();
            });

            window.on("close", (event) => {
                if (this.markQuit) {
                    return;
                }
                event.preventDefault();
                window.hide();
                return false;
            });

            if (global.DEBUG) {
                window.webContents.openDevTools();
            }
        }
    }

    public getWindow(window: string) {
        return this.windows.get(window) || null;
    }

    public createMainWindow() {
        const options: Electron.BrowserWindowConstructorOptions = {
            height: 600,
            width: 800,
            show: false,
            webPreferences: {
                nodeIntegration: true,
                contextIsolation: false,
            },
        };
        const mainWindow = this.createWindow(AppWindows.main, options);
        this.mainWindow = mainWindow;
        return mainWindow;
    }

    public openMainWindow() {
        let mainWindow = this.mainWindow;
        if (!mainWindow) {
            mainWindow = this.createMainWindow();
        }
        mainWindow!.show();
        mainWindow!.focus();
    }

    public closeMainWindow() {
        const mainWindow = this.mainWindow;
        if (mainWindow) {
            mainWindow.close();
        }
    }

    public startOverlay() {
        this.Overlay!.start();
        this.Overlay!.setHotkeys([
            {
                name: "overlay.hotkey.toggleSetting",
                keyCode: 0x70,
                modifiers: { alt: true },
            },
        ]);

        this.Overlay!.setEventCallback((event: string, payload: any) => {
            if (event === "game.input") {
                const window = BrowserWindow.fromId(payload.windowId);
                if (window) {
                    const intpuEvent = this.Overlay!.translateInputEvent(payload);

                    if (intpuEvent) {
                        if ("x" in intpuEvent) intpuEvent["x"] = Math.round(intpuEvent["x"] / this.scaleFactor);
                        if ("y" in intpuEvent) intpuEvent["y"] = Math.round(intpuEvent["y"] / this.scaleFactor);
                        window.webContents.sendInputEvent(intpuEvent);
                    }
                }
            } else if (event === "graphics.fps") {
                const window = this.getWindow("StatusBar");
                if (window) {
                    window.webContents.send("fps", payload.fps);
                }
            } else if (event === "game.hotkey.down") {
                if (payload.name === "overlay.hotkey.toggleSetting") {
                    console.log("toggleSetting");
                }
            } else if (event === "game.window.focused") {
                console.log("focusWindowId", payload.focusWindowId);

                BrowserWindow.getAllWindows().forEach((window) => {
                    window.blurWebView();
                });

                const focusWin = BrowserWindow.fromId(payload.focusWindowId);
                if (focusWin) {
                    focusWin.focusOnWebView();
                }
            }
        });
    }

    public addOverlayWindow(
        name: string,
        window: Electron.BrowserWindow,
        dragborder: number = 0,
        captionHeight: number = 0,
        transparent: boolean = false
    ) {
        const display = screen.getDisplayNearestPoint(screen.getCursorScreenPoint());

        this.Overlay!.addWindow(window.id, {
            name,
            transparent,
            resizable: window.isResizable(),
            maxWidth: window.isResizable ? display.bounds.width : window.getBounds().width,
            maxHeight: window.isResizable ? display.bounds.height : window.getBounds().height,
            minWidth: window.isResizable ? 100 : window.getBounds().width,
            minHeight: window.isResizable ? 100 : window.getBounds().height,
            nativeHandle: window.getNativeWindowHandle().readUInt32LE(0),
            rect: {
                x: window.getBounds().x,
                y: window.getBounds().y,
                width: Math.floor(window.getBounds().width * this.scaleFactor),
                height: Math.floor(window.getBounds().height * this.scaleFactor),
            },
            caption: {
                left: Math.floor(dragborder * this.scaleFactor),
                right: Math.floor(dragborder * this.scaleFactor),
                top: Math.floor(dragborder * this.scaleFactor),
                height: Math.floor(captionHeight * this.scaleFactor),
            },
            dragBorderWidth: Math.floor(dragborder),
        });

        window.webContents.on("paint", (event, dirty, image: Electron.NativeImage) => {
            if (this.markQuit) {
                return;
            }
            this.Overlay!.sendFrameBuffer(window.id, image.getBitmap(), image.getSize().width, image.getSize().height);
        });

        window.on("ready-to-show", () => {
            window.focusOnWebView();
        });

        window.on("resize", () => {
            console.log(`${name} resizing`);
            this.Overlay!.sendWindowBounds(window.id, {
                rect: {
                    x: window.getBounds().x,
                    y: window.getBounds().y,
                    width: Math.floor(window.getBounds().width * this.scaleFactor),
                    height: Math.floor(window.getBounds().height * this.scaleFactor),
                },
            });
        });

        // window.on("move", () => {
        //   this.Overlay!.sendWindowBounds(window.id, {
        //     rect: {
        //       x: window.getBounds().x,
        //       y: window.getBounds().y,
        //       width: Math.floor(window.getBounds().width * this.scaleFactor),
        //       height: Math.floor(window.getBounds().height * this.scaleFactor),
        //     },
        //   });
        // });

        const windowId = window.id;
        window.on("closed", () => {
            this.Overlay!.closeWindow(windowId);
        });

        window.webContents.on("cursor-changed", (event, type) => {
            let cursor;
            switch (type) {
                case "default":
                    cursor = "IDC_ARROW";
                    break;
                case "pointer":
                    cursor = "IDC_HAND";
                    break;
                case "crosshair":
                    cursor = "IDC_CROSS";
                    break;
                case "text":
                    cursor = "IDC_IBEAM";
                    break;
                case "wait":
                    cursor = "IDC_WAIT";
                    break;
                case "help":
                    cursor = "IDC_HELP";
                    break;
                case "move":
                    cursor = "IDC_SIZEALL";
                    break;
                case "nwse-resize":
                    cursor = "IDC_SIZENWSE";
                    break;
                case "nesw-resize":
                    cursor = "IDC_SIZENESW";
                    break;
                case "ns-resize":
                    cursor = "IDC_SIZENS";
                    break;
                case "ew-resize":
                    cursor = "IDC_SIZEWE";
                    break;
                case "none":
                    cursor = "";
                    break;
            }
            this.Overlay!.sendCommand({ command: "cursor", cursor });
        });
    }

    private createWindow(name: string, option: Electron.BrowserWindowConstructorOptions) {
        const window = new BrowserWindow(option);
        this.windows.set(name, window);
        window.on("closed", () => {
            this.windows.delete(name);
        });
        window.webContents.on("new-window", (e, url) => {
            e.preventDefault();
            shell.openExternal(url);
        });

        if (global.DEBUG) {
            window.webContents.on("before-input-event", (event: Electron.Event, input: Electron.Input) => {
                if (input.key === "F12" && input.type === "keyDown") {
                    window.webContents.openDevTools();
                }
            });
        }

        return window;
    }

    public closeAllWindows() {
        const windows = this.windows.values();
        for (const window of windows) {
            window.close();
        }
    }

    public closeWindow(name: string) {
        const window = this.windows.get(name);
        if (window) {
            window.close();
        }
    }

    public hideWindow(name: string) {
        const window = this.windows.get(name);
        if (window) {
            window.hide();
        }
    }

    public showAndFocusWindow(name: string) {
        const window = this.windows.get(name);
        if (window) {
            window.show();
            window.focus();
        }
    }

    public start() {
        this.createMainWindow();

        this.setupIpc();
    }

    public activate() {
        this.openMainWindow();
    }

    public quit() {
        this.markQuit = true;
        this.closeMainWindow();
        this.closeAllWindows();

        if (this.Overlay) {
            this.Overlay.stop();
        }
    }

    public createOsrStatusbarWindow() {
        const options: Electron.BrowserWindowConstructorOptions = {
            x: 100,
            y: 200,
            height: 50,
            width: 200,
            frame: false,
            show: false,
            transparent: true,
            resizable: false,
            backgroundColor: "#00000000",
            webPreferences: {
                offscreen: true,
                nodeIntegration: true,
                contextIsolation: false,
            },
        };

        const name = "StatusBar";
        const window = this.createWindow(name, options);

        // window.webContents.openDevTools({
        //   mode: "detach"
        // })
        window.loadURL(fileUrl(path.join(global.CONFIG.distDir, "index/statusbar.html")));

        this.addOverlayWindow(name, window, 0, 0);
        return window;
    }

    private setupIpc() {
        ipcMain.once("start", () => {
            this.scaleFactor = screen.getDisplayNearestPoint({
                x: 0,
                y: 0,
            }).scaleFactor;

            console.log(`starting overlay...`);
            this.startOverlay();

            // this.createOsrWindow();
            this.createOsrStatusbarWindow();
        });

        ipcMain.on("inject", (event, arg) => {
            console.log(`--------------------\n try inject ${arg}`);
            for (const window of this.Overlay.getTopWindows()) {
                if (window.title.indexOf(arg) !== -1) {
                    console.log(`--------------------\n injecting ${JSON.stringify(window)}`);
                    this.Overlay.injectProcess(window);
                }
            }
        });

        ipcMain.on("startIntercept", () => {
            this.Overlay!.sendCommand({
                command: "input.intercept",
                intercept: true,
            });
        });

        ipcMain.on("stopIntercept", () => {
            this.Overlay!.sendCommand({
                command: "input.intercept",
                intercept: false,
            });
        });
    }
}

export { IngameOverlay };
