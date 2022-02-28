echo "copy dlls to node-addon dir"
copy /y .\game-overlay\bin\Release\n_overlay.dll .\node-ovhook\build\Release
copy /y .\game-overlay\bin\Release\n_overlay.x64.dll .\node-ovhook\build\Release
copy /y .\game-overlay\bin\Release\n_ovhelper.exe .\node-ovhook\build\Release
copy /y .\game-overlay\bin\Release\n_ovhelper.x64.exe .\node-ovhook\build\Release
