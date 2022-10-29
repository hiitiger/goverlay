echo "copy dlls to node-addon dir"
copy /y .\game-overlay\prebuilt\n_overlay.dll .\electron-overlay\build\Release
copy /y .\game-overlay\prebuilt\n_overlay.x64.dll .\electron-overlay\build\Release
copy /y .\game-overlay\prebuilt\injector_helper.exe .\electron-overlay\build\Release
copy /y .\game-overlay\prebuilt\injector_helper.x64.exe .\electron-overlay\build\Release
