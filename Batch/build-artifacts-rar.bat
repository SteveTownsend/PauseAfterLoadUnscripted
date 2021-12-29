@if exist "S:\github\x64\release\Pause After Unload Unscripted SE-x.x.x.x.rar" (del "S:\github\x64\release\Pause After Unload Unscripted SE-x.x.x.x.rar")

@"C:\Program Files\WinRAR\rar.exe" a -ep -apSKSE/Plugins "s:/github/x64/release/Pause After Unload Unscripted SE-x.x.x.x.rar" "s:/github/x64/release/PauseAfterLoadUnscripted.dll"
@"C:\Program Files\WinRAR\rar.exe" a -ep -apSKSE/Plugins "s:/github/x64/release/Pause After Unload Unscripted SE-x.x.x.x.rar" s:/github/PauseAfterLoadUnscripted/Config/PauseAfterLoadUnscripted.ini

@"C:\Program Files\WinRAR\rar.exe" a -ep "s:/github/x64/release/Pause After Unload Unscripted SE-x.x.x.x.rar" s:/github/PauseAfterLoadUnscripted/LICENSE
