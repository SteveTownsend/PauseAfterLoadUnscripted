@if exist "S:\github\x64\release AE\Pause After Unload Unscripted AE-x.x.x.x.rar" (del "S:\github\x64\release AE\Pause After Unload Unscripted AE-x.x.x.x.rar")

@"C:\Program Files\WinRAR\rar.exe" a -ep -apSKSE/Plugins "j:/github/x64/release AE/Pause After Unload Unscripted AE-x.x.x.x.rar" "j:/github/x64/release AE/PauseAfterLoadUnscripted.dll"
@"C:\Program Files\WinRAR\rar.exe" a -ep -apSKSE/Plugins "j:/github/x64/release AE/Pause After Unload Unscripted AE-x.x.x.x.rar" j:/github/PauseAfterLoadUnscripted/Config/PauseAfterLoadUnscripted.ini

@"C:\Program Files\WinRAR\rar.exe" a -ep "j:/github/x64/release AE/Pause After Unload Unscripted AE-x.x.x.x.rar" j:/github/PauseAfterLoadUnscripted/LICENSE
