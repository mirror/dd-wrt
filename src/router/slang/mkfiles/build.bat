@echo off

if x%1 == x goto noparam

@echo.
@echo This script assumes you are working from a DOS shell, with NMAKE
@echo.

echo - generating Makefile in src...
src\mkfiles\mkmake %1 %2 %3 %4 %5 %6 %7 %8 %9 < src\mkfiles\makefile.all > src\Makefile

echo - generating Makefile in slsh...
src\mkfiles\mkmake %1 %2 %3 %4 %5 %6 %7 %8 %9 < slsh\mkfiles\makefile.all > slsh\Makefile

echo - generating Makefile in modules...
src\mkfiles\mkmake %1 %2 %3 %4 %5 %6 %7 %8 %9 < modules\mkfiles\makefile.all > modules\Makefile

copy mkfiles\makefile.dos Makefile
nmake
goto exit

:noparam
echo ERROR: need some parameters!

:exit
