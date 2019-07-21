import { BrowserWindow, ipcMain } from "electron"
import { Menu, Tray } from "electron"
import { screen, shell } from "electron"
import * as fs from "fs"
import * as path from "path"
import { fileUrl } from "../utils/utils"

import * as IOverlay from "electron-overlay"

import * as IOVhook from "node-ovhook"

enum AppWindows {
  main = "main",
  osr = "osr",
  osrpopup = "osrpopup"
}

class Application {
  private windows: Map<string, Electron.BrowserWindow>
  private tray: Electron.Tray | null
  private markQuit = false

  private Overlay: typeof IOverlay
  private OvHook: typeof IOVhook

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

  public getWindow(window: string) {
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

  public startOverlay() {
    this.Overlay = require("electron-overlay")
    this.Overlay!.start()
    this.Overlay!.setHotkeys([
      { name: "overlay.toggle", keyCode: 113, modifiers: { ctrl: true } },
      { name: "app.doit", keyCode: 114, modifiers: { ctrl: true } }
    ])

    this.Overlay!.setEventCallback((event: string, payload: any) => {
      if (event === "game.input") {
        const window = BrowserWindow.fromId(payload.windowId)
        if (window) {
          const intpuEvent = this.Overlay!.translateInputEvent(payload)
          // if (payload.msg !== 512) {
          //   console.log(event, payload)
          //   console.log(`translate ${JSON.stringify(intpuEvent)}`)
          // }

          if (intpuEvent) {
            window.webContents.sendInputEvent(intpuEvent)
          }
        }
      } else if (event === "graphics.fps") {
        const window = this.getWindow("StatusBar")
        if (window) {
          window.webContents.send("fps", payload.fps)
        }
      } else if (event === "game.hotkey.down") {
        if (payload.name === "app.doit") {
          this.doit()
        }
      } else if (event === "game.window.focused") {
        console.log("focusWindowId", payload.focusWindowId)

        BrowserWindow.getAllWindows().forEach((window) => {
          window.blurWebView()
        })

        const focusWin = BrowserWindow.fromId(payload.focusWindowId)
        if (focusWin) {
          focusWin.focusOnWebView()
        }
      }
    })
  }

  public addOverlayWindow(
    name: string,
    window: Electron.BrowserWindow,
    dragborder: number = 0,
    captionHeight: number = 0,
    transparent: boolean = false
  ) {
    const display = screen.getDisplayNearestPoint(
      screen.getCursorScreenPoint()
    )

    this.Overlay!.addWindow(window.id, {
      name,
      transparent,
      resizable: window.isResizable(),
      maxWidth: window.isResizable
        ? display.bounds.width
        : window.getBounds().width,
      maxHeight: window.isResizable
        ? display.bounds.height
        : window.getBounds().height,
      minWidth: window.isResizable ? 100 : window.getBounds().width,
      minHeight: window.isResizable ? 100 : window.getBounds().height,
      nativeHandle: window.getNativeWindowHandle().readUInt32LE(0),
      rect: {
        ...window.getBounds()
      },
      caption: {
        left: dragborder,
        right: dragborder,
        top: dragborder,
        height: captionHeight
      },
      dragBorderWidth: dragborder
    })

    window.webContents.on(
      "paint",
      (event, dirty, image: Electron.NativeImage) => {
        if (this.markQuit) {
          return
        }
        this.Overlay!.sendFrameBuffer(
          window.id,
          image.getBitmap(),
          image.getSize().width,
          image.getSize().height
        )
      }
    )

    window.on("ready-to-show", () => {
      window.focusOnWebView()
    })

    window.on("resize", () => {
      this.Overlay!.sendWindowBounds(window.id, { rect: window.getBounds() })
    })

    window.on("move", () => {
      this.Overlay!.sendWindowBounds(window.id, { rect: window.getBounds() })
    })

    const windowId = window.id
    window.on("closed", () => {
      this.Overlay!.closeWindow(windowId)
    })

    window.webContents.on("cursor-changed", (event, type) => {
      let cursor
      switch (type) {
        case "default":
          cursor = "IDC_ARROW"
          break
        case "pointer":
          cursor = "IDC_HAND"
          break
        case "crosshair":
          cursor = "IDC_CROSS"
          break
        case "text":
          cursor = "IDC_IBEAM"
          break
        case "wait":
          cursor = "IDC_WAIT"
          break
        case "help":
          cursor = "IDC_HELP"
          break
        case "move":
          cursor = "IDC_SIZEALL"
          break
        case "nwse-resize":
          cursor = "IDC_SIZENWSE"
          break
        case "nesw-resize":
          cursor = "IDC_SIZENESW"
          break
        case "ns-resize":
          cursor = "IDC_SIZENS"
          break
        case "ew-resize":
          cursor = "IDC_SIZEWE"
          break
        case "none":
          cursor = ""
          break
      }
      if (cursor) {
        this.Overlay!.sendCommand({ command: "cursor", cursor })
      }
    })
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

    window.setPosition(0, 0)
    // window.webContents.openDevTools({
    //   mode: "detach"
    // })
    window.loadURL(fileUrl(path.join(global.CONFIG.distDir, "index/osr.html")))

    window.webContents.on(
      "paint",
      (event, dirty, image: Electron.NativeImage) => {
        if (this.markQuit) {
          return
        }
        this.mainWindow!.webContents.send("osrImage", {
          image: image.toDataURL()
        })
      }
    )

    this.addOverlayWindow("MainOverlay", window, 10, 40)
    return window
  }

  public createOsrStatusbarWindow() {
    const options: Electron.BrowserWindowConstructorOptions = {
      height: 50,
      width: 200,
      frame: false,
      show: false,
      transparent: true,
      resizable: false,
      backgroundColor: "#00000000",
      webPreferences: {
        offscreen: true
      }
    }

    const name = "StatusBar"
    const window = this.createWindow(name, options)

    window.setPosition(100, 0)
    // window.webContents.openDevTools({
    //   mode: "detach"
    // })
    window.loadURL(
      fileUrl(path.join(global.CONFIG.distDir, "index/statusbar.html"))
    )

    this.addOverlayWindow(name, window, 0, 0)
    return window
  }

  public createOsrTipWindow() {
    const options: Electron.BrowserWindowConstructorOptions = {
      height: 220,
      width: 320,
      resizable: false,
      frame: false,
      show: false,
      transparent: true,
      webPreferences: {
        offscreen: true
      }
    }

    const getRandomInt = (min: number, max: number) => {
      return Math.floor(Math.random() * (max - min + 1)) + min
    }
    const name = `osrtip ${getRandomInt(1, 10000)}`
    const window = this.createWindow(name, options)

    window.setPosition(0, 0)
    // window.webContents.openDevTools({
    //   mode: "detach"
    // })
    window.loadURL(
      fileUrl(path.join(global.CONFIG.distDir, "index/osrtip.html"))
    )

    this.addOverlayWindow(name, window, 30, 40, true)
    return window
  }

  public closeAllWindows() {
    const windows = this.windows.values()
    for (const window of windows) {
      window.close()
    }
  }

  public closeWindow(name: string) {
    const window = this.windows.get(name)
    if (window) {
      window.close()
    }
  }

  public hideWindow(name: string) {
    const window = this.windows.get(name)
    if (window) {
      window.hide()
    }
  }

  public showAndFocusWindow(name: string) {
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

    this.setupSystemTray()

    this.setupIpc()
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

    if (this.Overlay) {
      this.Overlay.stop()
    }
  }

  public openLink(url: string) {
    shell.openExternal(url)
  }

  private createWindow(
    name: string,
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

    if (global.DEBUG) {
      window.webContents.on(
        "before-input-event",
        (event: Electron.Event, input: Electron.Input) => {
          if (input.key === "F12" && input.type === "keyDown") {
            window.webContents.openDevTools()
          }
        }
      )
    }

    return window
  }

  private setupIpc() {
    ipcMain.once("start", () => {
      if (!this.Overlay) {
        this.startOverlay()

        this.createOsrWindow()
        this.createOsrStatusbarWindow()
      }

      if (!this.OvHook) {
        this.OvHook = require("node-ovhook")
      }
    })

    ipcMain.on("inject", (event, arg) => {
      console.log(`--------------------\n try inject ${arg}`)
      for (const window of this.OvHook.getTopWindows()) {
        if (window.title.indexOf(arg) !== -1) {
          this.OvHook.injectProcess(window)
        }
      }
    })

    ipcMain.on("osrClick", () => {
      this.createOsrTipWindow()
    })

    ipcMain.on("doit", () => {
      this.doit()
    })

    ipcMain.on("startIntercept", () => {
      this.Overlay!.sendCommand({
        command: "input.intercept",
        intercept: true
      })
    })

    ipcMain.on("stopIntercept", () => {
      this.Overlay!.sendCommand({
        command: "input.intercept",
        intercept: false
      })
    })
  }

  private doit() {
    const name = "OverlayTip"
    this.closeWindow(name)

    const display = screen.getDisplayNearestPoint(
      screen.getCursorScreenPoint()
    )

    const window = this.createWindow(name, {
      width: 480,
      height: 270,
      frame: false,
      show: false,
      transparent: true,
      resizable: false,
      x: 0,
      y: 0,
      webPreferences: {
        offscreen: true
      }
    })

    this.addOverlayWindow(name, window, 0, 0)

    // window.webContents.openDevTools({mode: "detach"})

    window.loadURL(
      fileUrl(path.join(global.CONFIG.distDir, "doit/index.html"))
    )
  }
}

export { Application }
