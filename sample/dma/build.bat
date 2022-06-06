@echo off
rem build with Wine: wine cmd /c build.bat
setlocal enabledelayedexpansion enableextensions
mkdir out
echo Make mdssub.bin ...
..\..\tools\sjasmplus.exe ..\..\src\mdssub.z80 --raw=out\mdssub.bin --lst=out\mdssub.lst >error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.zx0 ...
..\..\tools\salvador.exe out\mdssub.bin out\mdssub.zx0
if %ERRORLEVEL% neq 0 goto error
echo Make main.bin ...
..\..\tools\asm68k.exe /k /p /o ae- main.68k, main.bin >error.txt, , main.lst
if %ERRORLEVEL% neq 0 goto error
echo All done!
goto eof
:error
type error.txt
pause
:eof
