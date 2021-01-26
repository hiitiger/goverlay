# game overlay

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/0f6b8ec919b243a7a926fcf674b2cab7)](https://www.codacy.com/app/hiitiger/gelectron?utm_source=github.com&utm_medium=referral&utm_content=hiitiger/gelectron&utm_campaign=Badge_Grade)
[![Build status](https://ci.appveyor.com/api/projects/status/sgi7go37f72f52a5?svg=true)](https://ci.appveyor.com/project/hiitiger/gelectron)

![demo](https://raw.githubusercontent.com/hiitiger/gelectron/master/screenshot/gelectron3.gif)

Use the power of web to inject any web app to overlay in your game

## Features

- electron offscreen window overlay in game
- dx11 api support
- dx10 api support
- dx9 api support
- multi windows support
- window z-index and focus
- in game sync drag and resize
- window draw policy
- input intercepting by manually control
- custom shaped window (alpha test for mouse handling)
- input intercepting by auto mouse check

## Requirements

- Visual Studio 2019 (C++ desktop workspace, winsdk 10.0.18362).
- Python 2 (`add to PATH`)

## Build

### node native-addons

For `electron-overlay` and `node-ovhook`

```bash
    cd client
    npm link ../electron-overlay
    npm link ../node-ovhook
    npm i
    npm run compile:electron
    npm run build
    npm run start
```

### Test apps

Visual Studio 2017 > gameoverlay.sln > Rebuild solution

## Test

1. build node projects
2. run demo client
3. click the start button to start overlay
4. build test apps
5. start dx11app and press ESC to do load inject dll for test
6. press F1 to switch input intercepting

## Inject a specific game

After you have the demo runs right

1. copy files [`n_overlay.dll`, `n_overlay.x64.dll`, `n_ovhelper.exe`, `n_ovhelper.x64.exe`] from directory `game-overlay\bin\Release` to directory `node-ovhook\build\Release`
2. run demo client
3. click start button to start overlay
4. start the game you want to inject to and wait for it game window to show
5. input the window title(or part of the title) of the game, and click the inject button

## Usage

Check doc/doc.md for usage in your own projects
