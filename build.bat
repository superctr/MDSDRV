@echo off
rem build with Wine: wine cmd /c build.bat
setlocal enabledelayedexpansion enableextensions
mkdir out
set MDSDATA=
for %%x in (data\bgm\*.mml,data\bgm\*.mds,data\se\*.mml,data\se\*.mds) do set MDSDATA=!MDSDATA! "%%x"
echo Found files: %MDSDATA%
echo Make mdsseq.bin and mdspcm.bin ...
tools\mdslink.exe -o "out/mdsseq.bin" "out/mdspcm.bin" -i "out/mdsseq.inc" -h "out/mdsseq.h" %MDSDATA% > out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.bin ...
tools\sjasmplus.exe src\mdssub.z80 --raw=out\mdssub.bin --lst=out\mdssub.lst >out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.zx0 ...
tools\salvador.exe out\mdssub.bin out\mdssub.zx0
if %ERRORLEVEL% neq 0 goto error
echo Make main.bin ...
tools\asm68k.exe /k /p /o ae- src\main.68k, main.bin >out\error.txt, , out\main.lst
if %ERRORLEVEL% neq 0 goto error
echo Make mdsdrv.bin ...
tools\asm68k.exe /k /p /o ae- src\blob.68k, out\mdsdrv.bin >out\error.txt, , out\mdsdrv.lst
if %ERRORLEVEL% neq 0 goto error
if "%GDK_WIN%"=="" goto nosgdk
echo Make sgdk.bin
copy /b out\mdsdrv.bin sample\sgdk\res
copy /b out\mdspcm.bin sample\sgdk\res
copy /b out\mdsseq.bin sample\sgdk\res
copy out\mdsseq.h sample\sgdk\res
pushd sample\sgdk
make clean
make
popd
copy sample\sgdk\out\rom.bin sgdk.bin
:nosgdk
echo All done!
goto eof
:error
type out\error.txt
pause
:eof
