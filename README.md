# game overlay solution for any desktop GUI like Electron, Qt and CEF⚡

fork of https://github.com/hiitiger/gelectron
use the power of web to inject any web app to overlay in your game

## screenshot

![demo](https://raw.githubusercontent.com/hiitiger/gelectron/master/screenshot/gelectron3.gif)

## NOTE

1. need Visual Sudio 2017 to build native C++ project.(2019 in works)
2. we can assocciate a new IME conext if game disabled it, but in fullscreen it will cause window to lose focus.

## run demo

1. git clone https://github.com/hiitiger/gelectron.git
2. run demo client
    - this includes the steps to build node native-addons `electron-overlay` and `node-ovhook`
    ```
        cd client
        npm link ../electron-overlay
        npm link ../node-ovhook
        npm i
        npm run compile:electron
        npm run build
        npm run start
    ```
3. click the start button to start overlay
4. start the game you want to inject to and wait for it game window to show
5. input the window title(or part of the title) of the game, and click the inject button
6. press ctrl+tab to open overlay

## compiling overlay and ovhelper on you own

In default they are precompiled under `client/dist/overlay` but if you are making changes you might want to compile on your own

1. open gameoverlay.sln and build all (Release/Win32 + Release/x64) or just run `build.bat` in game-overlay directory
2. copy files [`n_overlay.dll`, `n_overlay.x64.dll`, `n_ovhelper.exe`, `n_ovhelper.x64.exe`] from directory `game-overlay\bin\Release` to directory `client/dist/overlay`

## use in your own project

1. checkout [document](https://github.com/hiitiger/gelectron/blob/master/doc/doc.md) about how to use it in your own project

## features & todos

Look into the Projects/Progress board
