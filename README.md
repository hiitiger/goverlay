# game overlay solution for any desktop GUI like Electron, Qt and CEF⚡

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/0f6b8ec919b243a7a926fcf674b2cab7)](https://www.codacy.com/app/hiitiger/gelectron?utm_source=github.com&utm_medium=referral&utm_content=hiitiger/gelectron&utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/sgi7go37f72f52a5?svg=true)](https://ci.appveyor.com/project/hiitiger/gelectron)

## game overlay for Electron

use the power of web to inject any web app to overlay in your game

## screenshot

![demo](https://raw.githubusercontent.com/hiitiger/gelectron/master/screenshot/gelectron3.gif)

## NOTE

1. need Visual Sudio 2017 to build native C++ project.
2. ~~use 32bit node to install 32bit electron (game overlay support x64 no matter electron version), if you want to use 64bit electron, modify compile:electron script in client/package.json.~~
3. we can assocciate a new IME conext if game disabled it, but in fullscreen it will cause window to lose focus.
4. **node-gyp** need **python2** installed on system and make sure it's added to PATH

## run demo

1. git clone https://github.com/hiitiger/gelectron.git
2. ~~git submodule update --init --recursive~~
3. ~~cd to boost and do boost bootstrap~~
4. run demo client
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
5. click the start button to start overlay
6. open gameoverlay.sln and build all (Release/Win32 + Release/x64) or just run `build.bat` in game-overlay directory
7. start dx11app and press ESC to do load inject dll for test
8. press F1 to swtich input intercepting

## inject a specific game

After you have the demo runs right

1.  copy files [`n_overlay.dll`, `n_overlay.x64.dll`, `n_ovhelper.exe`, `n_ovhelper.x64.exe`] from directory `game-overlay\bin\Release` to directory `node-ovhook\build\Release`
2.  run demo client
3.  click start button to start overlay
4.  start the game you want to inject to and wait for it game window to show
5.  input the window title(or part of the title) of the game, and click the inject button

## use in your own project

1. checkout [document](https://github.com/hiitiger/gelectron/blob/master/doc/doc.md) about how to use it in your own project

## feature

-   [x] electron offscreen window overlay in game
-   [x] dx12 api support
-   [x] dx11 api support
-   [x] dx10 api support
-   [x] dx9 api support
-   [x] multi windows support
-   [x] window z-index and focus
-   [x] in game sync drag and resize
-   [ ] in game defered drag and resize
-   [x] window draw policy
-   [x] input intercepting by manually control
-   [x] custom shaped window (alpha test for mouse handling)
-   [ ] detailed injecting configuration
-   [x] input intercepting by auto mouse check

## todo

-   [ ] browser window state manage
-   [x] better hotkey
-   [x] session reconnect
