import { BrowserWindow, ipcMain } from "electron"
import { Menu, Tray } from "electron"
import { screen, shell } from "electron"
import * as fs from "fs"
import * as path from "path"
import { fileUrl } from "../utils/utils"

import * as IOverlay from "electron-overlay"

enum AppWindows {
  main = "main",
  osr = "osr"
}

class Application {
  private windows: Map<string, Electron.BrowserWindow>
  private tray: Electron.Tray | null
  private markQuit = false

  private Overlay?: typeof IOverlay

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

  public startOverlay() {
    this.Overlay = require("electron-overlay")!
    this.Overlay!.start()

    this.Overlay!.setEventCallback((event: string, payload: any) => {
      if (event === "game.input") {
        const osrwindow = this.getWindow(AppWindows.osr)
        if (osrwindow) {
          const intpuEvent = this.Overlay!.translateInputEvent(payload)
          if (payload.msg !== 512) {
            console.log(event, payload)
            console.log(`translate ${JSON.stringify(intpuEvent)}`)
          }
          osrwindow.webContents.sendInputEvent(intpuEvent)
        }
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
      backgroundColor: "#f0ffffff",
      webPreferences: {
        offscreen: true
      }
    }

    const window = this.createWindow(AppWindows.osr, options)
    window.webContents.on(
      "paint",
      (event, dirty, image: Electron.NativeImage) => {
        if (this.markQuit) {
          return
        }
        this.mainWindow!.webContents.send("osrImage", {
          image: image.toDataURL()
        })

        this.Overlay!.sendFrameBuffer(window.id, image.getBitmap(), image.getSize().width, image.getSize().height)
      }
    )

    window.webContents.openDevTools({
      mode: "detach"
    })
    window.loadURL(fileUrl(path.join(global.CONFIG.distDir, "index/osr.html")))

    this.Overlay!.addWindow(window.id, {
      name: "MainOverlay",
      transparent: false,
      resizable: true,
      maxWidth: 1920,
      maxHeight: 1920,
      minWidth : 100,
      minHeight: 100,
      nativeHandle: window.getNativeWindowHandle().readUInt32LE(0),
      rect: {
          ...window.getBounds(),
      },
      caption: {
        left: 10,
        right: 10,
        top: 10,
        height: 40
      },
      dragBorderWidth: 10
    })

    window.on("ready-to-show", () => {
      window.focusOnWebView()
    })

    window.on("resize", () => {
      this.Overlay!.sendWindowBounds(window.id, {rect: window.getBounds()})
    })

    window.on("move", () => {
      console.log("move", window.getBounds())
      this.Overlay!.sendWindowBounds(window.id, {rect: window.getBounds()})
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
        case "none":
          cursor = ""
          break
      }
      if (cursor) {
        this.Overlay!.sendCommand( { command: "cursor", cursor })
      }
    })

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

    this.setupSystemTray()

    this.setupIpc()

    this.startOverlay()
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
    ipcMain.on("click", () => {
      this.createOsrWindow().setPosition(0, 0)
    })
  }
}

export { Application }
