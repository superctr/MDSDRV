@echo off
REM apps/asm68k.exe /k /p /o ae- ressrc\hwf.asm, res\MEGAOS\UI\HWF.HWF >results\errors.txt, , listings\fnt.lst
asm68k.exe /k /p /o ae- main.68k, main.bin >error.txt, , main.lst
if %ERRORLEVEL% neq 0 goto error
asm68k.exe /k /p /o ae- blob.68k, mdsdrv.bin >error.txt, , mdsdrv.lst
if %ERRORLEVEL% neq 0 goto error
goto eof
:error
type error.txt
pause
:eof
