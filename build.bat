@echo off
rem build with Wine: wine cmd /c build.bat
mkdir out
echo Make mdsseq.bin and mdspcm.bin ...
tools\mdslink.exe -o "out/mdsseq.bin" "out/mdspcm.bin" -i "out/mdsseq.inc" -h "out/mdsseq.h" ^
  "data/bgm/jazzy_nyc_99.mml" ^
  "data/bgm/idk.mds" ^
  "data/bgm/sand_light.mds" ^
  "data/bgm/junkers_high.mds" ^
  "data/bgm/midnight.mds" ^
  "data/bgm/passport.mds" ^
  "data/se/beep1.mml" ^
  "data/se/beep2.mml" ^
  "data/se/beep3.mml" ^
  "data/se/beep4.mml" ^
  "data/se/explosion1.mml" ^
  "data/se/explosion2.mml" ^
  "data/se/explosion3.mml" ^
  "data/se/menu1.mml" ^
  "data/se/menu2.mml" ^
  "data/se/menu3.mml" ^
  "data/se/noise1.mml" ^
  "data/se/noise2.mml" ^
  "data/se/pcm1.mml" ^
  "data/se/pcm2.mml" ^
  > out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.bin ...
tools\sjasmplus.exe src\mdssub.z80 --raw=out\mdssub.bin --lst=out\mdssub.lst >out\error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.slz ...
tools\slz.exe -c out\mdssub.bin out\mdssub.slz
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
