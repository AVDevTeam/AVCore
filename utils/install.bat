xcopy /Y AVCommDriver.sys %WINDIR%\System32\DRIVERS\
sc create injdrv binPath= %cd%\injdrv.sys type= kernel
sc create AVCore binPath= %cd%\AVCore.exe start= demand