@echo off
PUSHD
if not exist ImageComp.exe cd ..
if not exist ImageComp.exe echo Err: Could not find ImageComp.exe & goto:end
SET REGDATA_PATH=Output/rose1/regData/rose1_40.regdata
if not exist %REGDATA_PATH% echo Err: Could not find target regdata file (%REGDATA_PATH%). Did you run testSimplify.bat first? & goto:end
@echo on
ImageComp ../TestFiles/rose1.jpg %REGDATA_PATH% ../TestFiles/waterPicLib CompOut/rose1_40_water.jpg
@echo off
:end
POPD
pause