xcopy /Y AVCommDriver.sys %WINDIR%\System32\DRIVERS\
sc create AVCore binPath= C:\users\user\desktop\AVCore\AVCore.exe start= demand