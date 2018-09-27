# game-overlay in electron ⚡

use the power of web to inject any web app to overlay in your game

## screenshot
![demo](https://raw.githubusercontent.com/hiitiger/gelectron/master/screenshot/gelectron.gif)

## run demo
1. git clone https://github.com/hiitiger/gelectron.git
2. git submodule update --init --recursive
3. cd to boost and do boost bootstrap
    ```
        .\bootstrap
        .\b2
    ```
4. cd to demo client 
    ```
        npm link ../electron-overlay
        npm i
        npm run compile:electron
        npm run build
        npm run start
    ```
5. open gameoverlay.sln and build all 
6. start dx11app and press ESC to do load inject dll for test
7. press F1 to swtich input intercepting

## feature
    - [x] electron offscreen window overlay in game
    - [ ] dx12 api support
    - [x] dx11 api support
    - [x] dx10 api support
    - [ ] dx9 api support
    - [x] multi windows support
    - [x] window z-index and focus
    - [x] in game sync drag and resize
    - [ ] in game defered drag and resize
    - [x] input intercepting
    - [ ] custom shaped window (alpha test for mouse handling)
    - [ ] detailed injecting configuration 

## todo
    - [ ] maximize to game window size