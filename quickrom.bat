@echo off
rem build with Wine: wine cmd /c build.bat
setlocal enabledelayedexpansion enableextensions
cd /d %~dp0
mkdir out
set MDSDATA=%1
echo Found files: %MDSDATA%
if not exist %MDSDATA% goto nofile
echo Found files: %MDSDATA%
echo Make mdsseq.bin and mdspcm.bin ...
tools\mdslink.exe -o "out/mdsseq.bin" "out/mdspcm.bin" -i "out/mdsseq.inc" -h "out/mdsseq.h" %MDSDATA% > out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.bin ...
tools\sjasmplus.exe src\mdssub.z80 --raw=out\mdssub.bin --lst=out\mdssub.lst >out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.slz ...
tools\slz.exe -c out\mdssub.bin out\mdssub.slz
if %ERRORLEVEL% neq 0 goto error
echo Make %~n1.bin ...
tools\asm68k.exe /k /p /o ae- src\main.68k, %~dpn1.bin >out\error.txt, , out\main.lst
if %ERRORLEVEL% neq 0 goto error
echo All done!
goto eof
:error
type out\error.txt
pause
goto eof
:nofile
echo Usage: quickrom [path_to_MDS].
echo You can drag and drop to generate a rom
pause
:eof