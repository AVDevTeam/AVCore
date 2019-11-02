xcopy /Y AVCommDriver.sys %WINDIR%\System32\DRIVERS\
sc create injdrv binPath= C:\users\user\desktop\AVCore\injdrv.sys type= kernel
sc create AVCore binPath= C:\users\user\desktop\AVCore\AVCore.exe start= demand