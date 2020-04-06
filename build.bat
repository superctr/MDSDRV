@echo off
tools\sjasmplus.exe mdssub.z80 --raw=mdssub.bin --lst=mdssub.lst >error.txt
if %ERRORLEVEL% neq 0 goto error
tools\asm68k.exe /k /p /o ae- main.68k, main.bin >error.txt, , main.lst
if %ERRORLEVEL% neq 0 goto error
tools\asm68k.exe /k /p /o ae- blob.68k, mdsdrv.bin >error.txt, , mdsdrv.lst
if %ERRORLEVEL% neq 0 goto error
goto eof
:error
type error.txt
pause
:eof
