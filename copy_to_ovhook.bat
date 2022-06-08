echo "copy dlls to node-addon dir"
copy /y .\game-overlay\prebuilt\n_overlay.dll .\node-ovhook\build\Release
copy /y .\game-overlay\prebuilt\n_overlay.x64.dll .\node-ovhook\build\Release
copy /y .\game-overlay\prebuilt\injector_helper.exe .\node-ovhook\build\Release
copy /y .\game-overlay\prebuilt\injector_helper.x64.exe .\node-ovhook\build\Release
