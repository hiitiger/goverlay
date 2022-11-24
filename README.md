# DirectX hook and game overlay solution for any desktop GUI like Electron, Qt, CEF and WPF⚡

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4fe290657a91448caecaa5583c84b9d1)](https://www.codacy.com/gh/hiitiger/goverlay/dashboard?utm_source=github.com&utm_medium=referral&utm_content=hiitiger/goverlay&utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/sgi7go37f72f52a5?svg=true)](https://ci.appveyor.com/project/hiitiger/goverlay)

## game overlay solution 
* based on DirectX hook, draw in game
* support any GUI framework, use the power of web/WPF/Qt to inject any app to overlay in your game
* easy window management
* input intercept

## screenshot

![demo](https://raw.githubusercontent.com/hiitiger/goverlay/master/screenshot/gelectron3.gif)

## Prerequisites

1. Visual Sudio 2019 with latest sdk to build native C++ project.
2. CMake
3. node 32bit/64bit

## run demo

1. git clone https://github.com/hiitiger/goverlay.git
2. run demo client
   - this includes the steps to build node native-addons `electron-overlay`
   ```CMD
       npm i
       npm run build
       @REM  for 32bit electron
       npm run build:addon:x86
       @REM  for 64bit electron
       npm run build:addon:x64

       npm run start
   ```
3. click the start button to start overlay
4. start a d3d game, type its window name and inject
4. `CTRL+F2` to toggle full input intercept, `CTRL+F3` to show demo popup(doit ^^)

## inject a specific game

After you have the demo runs right

1.  copy files [`n_overlay.dll`, `n_overlay.x64.dll`, `injector_helper.exe`, `injector_helper.x64.exe`] from directory `game-overlay\prebuilt` to directory `electron-overlay`
2.  run demo client
3.  click start button to start overlay
4.  start the game you want to inject to and wait for it game window to show
5.  input the window title(or part of the title) of the game, and click the inject button

## use in your own project

1. checkout [document](https://github.com/hiitiger/gelectron/blob/master/doc/doc.md) about how to use it in your own project

## feature

- [x] electron offscreen window overlay in game
- [x] dx12 api support
- [x] dx11 api support
- [x] dx10 api support
- [x] dx9 api support
- [ ] OpenGL api support
- [ ] native draw overlay
- [ ] hardware acc osr bitmap transport
- [x] multi windows support
- [x] window z-index and focus
- [x] in game sync drag and resize
- [ ] in game defered drag and resize
- [x] window draw policy
- [x] input intercepting by manually control
- [x] custom shaped window (alpha test for mouse handling)
- [x] input intercepting by auto mouse check
- [x] brwoser window state manage
- [x] better hotkey
- [x] session reconnect

## support

contact me if had issues with specific features or in-game performance.

contact me for other GUI frameworks support or need any special feature.

## note

Many games block dll injection, please sign dll files with your certificate.

## products using goverlay

- FACEIT: https://www.faceit.com/, competitive gaming platform
- Guilded: https://www.guilded.gg/, game chat
- OP.GG: https://www.op.gg/, game statistic platform
- senpai.gg: https://senpai.gg/, game statistic and assistant
- Medal.tv: https://medal.tv/, game recording software