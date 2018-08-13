import { BrowserWindow } from "electron"
import { Menu, Tray } from "electron"
import { shell } from "electron"
import * as fs from "fs"
import * as path from "path"
import { fileUrl } from "../utils/utils"

enum AppWindows {
    main = "main",
    osr = "osr"
}

class Application {
    private windows: Map<string, Electron.BrowserWindow>
    private tray: Electron.Tray | null
    private markQuit = false

    constructor() {
        this.windows = new Map()
        this.tray = null
    }

    get mainWindow() {
        return this.windows.get(AppWindows.main) || null
    }

    set mainWindow(window: Electron.BrowserWindow | null) {
        if (!window) {
            this.windows.delete(AppWindows.main)
        } else {
            this.windows.set(AppWindows.main, window)
            window.on("closed", () => {
                this.mainWindow = null
            })

            window.loadURL(global.CONFIG.entryUrl)

            window.on("ready-to-show", () => {
                this.showAndFocusWindow(AppWindows.main)
            })

            window.webContents.on("did-fail-load", () => {
                window.reload()
            })

            window.on("close", (event) => {
                if (this.markQuit) {
                    return
                }
                event.preventDefault()
                window.hide()
                return false
            })

            if (global.DEBUG) {
                window.webContents.openDevTools()
            }
        }
    }

    public getWindow(window: AppWindows) {
        return this.windows.get(window) || null
    }

    public createMainWindow() {
        const options: Electron.BrowserWindowConstructorOptions = {
            height: 600,
            width: 800,
            show: false
        }
        const mainWindow = this.createWindow(AppWindows.main, options)
        this.mainWindow = mainWindow
        return mainWindow
    }

    public openMainWindow() {
        let mainWindow = this.mainWindow
        if (!mainWindow) {
            mainWindow = this.createMainWindow()
        }
        mainWindow!.show()
        mainWindow!.focus()
    }

    public closeMainWindow() {
        const mainWindow = this.mainWindow
        if (mainWindow) {
            mainWindow.close()
        }
    }

    public createOsrWindow() {
        const options: Electron.BrowserWindowConstructorOptions = {
            height: 360,
            width: 640,
            frame: false,
            show: false,
            transparent: true,
            webPreferences: {
                offscreen: true
            }
        }
        const window = this.createWindow(AppWindows.osr, options)
        window.webContents.on(
            "paint",
            (event, dirty, image: Electron.NativeImage) => {
                console.log("paint....")
                fs.writeFile(`orsex${Date.now()}.png`, image.toPNG(), (err) => {
                    if (err) {
                        throw err
                    }
                    console.log("The file has been saved!")
                })
                this.mainWindow!.webContents.send("osrImage", {
                    image: image.toDataURL()
                })
            }
        )

        window.loadURL(
            fileUrl(path.join(global.CONFIG.distDir, "index/osr.html"))
        )
        return window
    }

    public closeAllWindows() {
        const windows = this.windows.values()
        for (const window of windows) {
            window.close()
        }
    }

    public closeWindow(name: AppWindows) {
        const window = this.windows.get(name)
        if (window) {
            window.close()
        }
    }

    public hideWindow(name: AppWindows) {
        const window = this.windows.get(name)
        if (window) {
            window.hide()
        }
    }

    public showAndFocusWindow(name: AppWindows) {
        const window = this.windows.get(name)
        if (window) {
            window.show()
            window.focus()
        }
    }

    public setupSystemTray() {
        if (!this.tray) {
            this.tray = new Tray(
                path.join(global.CONFIG.distDir, "assets/icon-16.png")
            )
            const contextMenu = Menu.buildFromTemplate([
                {
                    label: "OpenMainWindow",
                    click: () => {
                        this.showAndFocusWindow(AppWindows.main)
                    }
                },
                {
                    label: "Quit",
                    click: () => {
                        this.quit()
                    }
                }
            ])
            this.tray.setToolTip("WelCome")
            this.tray.setContextMenu(contextMenu)

            this.tray.on("click", () => {
                this.showAndFocusWindow(AppWindows.main)
            })
        }
    }

    public start() {
        this.createMainWindow()
        this.createOsrWindow()
        this.setupSystemTray()
    }

    public activate() {
        this.openMainWindow()
    }

    public quit() {
        this.markQuit = true
        this.closeMainWindow()
        this.closeAllWindows()
        if (this.tray) {
            this.tray.destroy()
        }
    }

    public openLink(url: string) {
        shell.openExternal(url)
    }

    private createWindow(
        name: AppWindows,
        option: Electron.BrowserWindowConstructorOptions
    ) {
        const window = new BrowserWindow(option)
        this.windows.set(name, window)
        window.on("closed", () => {
            this.windows.delete(name)
        })
        window.webContents.on("new-window", (e, url) => {
            e.preventDefault()
            shell.openExternal(url)
        })
        return window
    }
}

export { Application }
