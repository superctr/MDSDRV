@echo off
rem build with Wine: wine cmd /c build.bat
echo Make mdsseq.bin and mdspcm.bin ...
tools\mdslink.exe -o mdsseq.bin mdspcm.bin ^
  "sample/jazzy_nyc_99.mml" ^
  "sample/idk.mds" ^
  "sample/sand_light.mds" ^
  "sample/junkers_high.mds" ^
  "sample/midnight.mds" ^
  "sample/astronomia.mds" ^
  "sample/passport.mds" ^
  "sample/pcmtest2.mds" ^
  "sample/pcmtest.mds" ^
  "sample/effect/beep1.mml" ^
  "sample/effect/beep2.mml" ^
  "sample/effect/beep3.mml" ^
  "sample/effect/beep4.mml" ^
  "sample/effect/explosion1.mml" ^
  "sample/effect/explosion2.mml" ^
  "sample/effect/explosion3.mml" ^
  "sample/effect/menu1.mml" ^
  "sample/effect/menu2.mml" ^
  "sample/effect/menu3.mml" ^
  "sample/effect/noise1.mml" ^
  "sample/effect/noise2.mml" ^
  > error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.bin ...
tools\sjasmplus.exe mdssub.z80 --raw=mdssub.bin --lst=mdssub.lst >error.txt
if %ERRORLEVEL% neq 0 goto error
echo Make mdssub.slz ...
tools\slz.exe -c mdssub.bin mdssub.slz
if %ERRORLEVEL% neq 0 goto error
echo Make main.bin ...
tools\asm68k.exe /k /p /o ae- main.68k, main.bin >error.txt, , main.lst
if %ERRORLEVEL% neq 0 goto error
echo Make mdsdrv.bin ...
tools\asm68k.exe /k /p /o ae- blob.68k, mdsdrv.bin >error.txt, , mdsdrv.lst
if %ERRORLEVEL% neq 0 goto error
echo All done!
goto eof
:error
type error.txt
pause
:eof
