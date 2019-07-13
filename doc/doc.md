# gelectron document

To enable an electron app to have the ability to show its web browser window in another game process, we need to injecting a module (dll in Windows) to the game, make an IPC connection with our electron process, copy the screenshot of our electron app's browser window and render it over the game's surface.

Then, since we also want our injected web page can reponse to user's input, we need to intercept the game's input so that we can pass the input to out electron app and send it the the browser window.

## modules we have

#### `n_overlay.dll`(`n_overlay.x64.dll` for x64)

This is the most important module (a dll), it will be injected into game process so that we can communicate with a game process and draw our own stuff on game window.

#### `n_ovhelper.exe`(`n_ovhelper.x64.exe` for x64)

This is a helper process which do the real dll injecting work for use.

#### `electron-overlay`

This is the node addon used in our electron app, use it to communicate with game process (`n_overlay.dll`), like sending electron webview framebuffer and recieve game input data.

#### `node-ovhook`

This is also a helper nodule, it's the node addon used it our electron app which uses `n_ovhelper.exe` to do injecting, it also can get the system's foreground window(we can check if it's the game window to decide when we will do injecting).

## how to use in your own project

so with the modules, basicly what we need to do is

0. make sure your compile x86 and x64 version native modules for the game you want to inejct
    1. build `game-overlay.sln` with Release config for x86 or x64 version, you'll get `n_ovhelper.exe` and `n_overlay.dll` for x86 version (or `n_ovhelper.x64.exe` and `n_overlay.x64.dll` for the x64 version)
    2. add `node-ovhook` and `electron-overlay` addons to your electron ap's dependency, nodejs should automatically build them, if not cd to their directory and build them manually.
    3. copy `n_ovhelper.exe` and `n_overlay.dll` to `node_modules/node-ovhook/build/Release` so node-ovhook can find them.
1. prepare a game
1. the electron app
    1. Create an electorn app
    2. import `electron-overlay` addon (as `IOverlay` for example),
        1. use `IOverlay.start()` to start the overlay server
        2. set up hotkeys and event callbacks (`game.input` is the most important one)
    3. create a transparent browser window (so we can capture it surface and pass it to the game)
        1. after create the transparent browser window, use `IOverlay.addWindow(...)` to add it to the overlay windows
        2. listen on its paint event and send the framebuffer to overlay use `IOverlay.sendFrameBuffer`
    4. on `game.input` event, translate the event to electron's format use `IOverlay.translateInputEvent`, and pass to electorn's window `window.webContents.sendInputEvent(inputEvent)`
    5. don't forget to handle window's `resize` events
1. do injecting.
    1. import `node-ovhook` addon (as `IOVhook` for example) to help us inject the `n_overlay.dll` module to the game process
    2. get the game process's process id and do injecting use `IOVhook.injectProcess`
1. if everything is ok ,you should see the injected browser window in game process

## features

#### hotkeys

check out hot the demo (client/src/main/electron/app-entry.ts) uses hotkeys by `this.Overlay!.setHotkeys`

#### cusomize window show/hide

if we look at the demo, we can find that on the topleft, a small browser window is always show in the game and other windows will show or hide responding to our hotkey.

Actually we can decide which window will always stay in game and which will only appear if we calls it.

Now, I do it in the `n_overlay` module, so if you want to do some customizing you need to change to code in n_overlay.
