# game-overlay in electron ⚡

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/0f6b8ec919b243a7a926fcf674b2cab7)](https://www.codacy.com/app/hiitiger/gelectron?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=hiitiger/gelectron&amp;utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/sgi7go37f72f52a5?svg=true)](https://ci.appveyor.com/project/hiitiger/gelectron)

use the power of web to inject any web app to overlay in your game

## screenshot
![demo](https://raw.githubusercontent.com/hiitiger/gelectron/master/screenshot/gelectron3.gif)


## NOTE
1. ~~If you have compile issues with boost(_MSVC_LANG), make sure you are using vs2017 15.7.x or newer.~~
2. use 32bit node to install 32bit electron (game overlay support x64 no matter electron version), if you want to use 64bit electron, modify compile:electron script in client/package.json.
3. we can assocciate a new IME conext if game disabled it, but in fullscreen it will cause window to lose focus.

## run demo
1. git clone https://github.com/hiitiger/gelectron.git
2. ~~git submodule update --init --recursive~~
3. ~~cd to boost and do boost bootstrap~~
4. cd to demo client 
    ```
        npm link ../electron-overlay
        npm i
        npm run compile:electron
        npm run build
        npm run start
    ```
5. click the start button to start 
6. open gameoverlay.sln and build all 
7. start dx11app and press ESC to do load inject dll for test
8. press F1 to swtich input intercepting

## feature
    - [x] electron offscreen window overlay in game
    - [ ] dx12 api support
    - [x] dx11 api support
    - [x] dx10 api support
    - [x] dx9 api support
    - [x] multi windows support
    - [x] window z-index and focus
    - [x] in game sync drag and resize
    - [ ] in game defered drag and resize
    - [x] window draw policy
    - [x] input intercepting by manually control
    - [x] custom shaped window (alpha test for mouse handling)
    - [ ] detailed injecting configuration 
    - [x] input intercepting by auto mouse check

## todo
    - [ ] brwoser window state manage
    - [x] better hotkey
    - [x] session reconnect



