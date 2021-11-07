if not exist "AggregateBuild" mkdir "AggregateBuild"
copy /B /Y ImageInterface\x64\Release\ImageInterface.dll AggregateBuild\ImageInterface.dll
copy /B /Y ImageComp\x64\Release\ImageComp.exe AggregateBuild\ImageComp.exe
copy /B /Y ImageSimplify\x64\Release\ImageSimplify.exe AggregateBuild\ImageSimplify.exe
pause