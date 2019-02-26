# gelectron document


## modules 

#### n_overlay.dll(n_overlay.x64.dll for x64)
this is the module(a dll) that will be injected into game process so that we can communicate with a game process and draw our own stuff on game window.

#### n_ovhelper.exe(n_ovhelper.x64.exe for x64)
this is the process that do the real dll inject work.

#### electron-overlay
this is the node addon used in our electron app, use it to communicate with game process(n_overlay.dll), like sending electron webview framebuffer.

#### node-ovhook
this is a node addon used it our electron app, it will use n_ovhelper.exe to do injecting, it also can get the system's foreground window(we can check if it's the game window to decide when we will do injecting).

## how to use in your own project