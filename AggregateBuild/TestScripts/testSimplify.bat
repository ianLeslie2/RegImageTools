@echo off
PUSHD
if not exist ImageSimplify.exe cd ..
if not exist ImageSimplify.exe echo Err: Could not find ImageSimplify.exe & goto:end
@echo on
ImageSimplify ../TestFiles/rose1.jpg
@echo off
:end
POPD
pause