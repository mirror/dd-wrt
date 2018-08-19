@echo off
echo - generating Makefile in src...
src\mkfiles\mkmake WIN32 MINGW32 DLL < src\mkfiles\makefile.all > src\Makefile

echo - generating Makefile in slsh...
src\mkfiles\mkmake WIN32 MINGW32 DLL < slsh\mkfiles\makefile.all > slsh\Makefile

echo - generating Makefile in modules...
src\mkfiles\mkmake WIN32 MINGW32 DLL < modules\mkfiles\makefile.all > modules\Makefile

copy mkfiles\makefile.m32 Makefile

@ echo.
@ echo Now run mingw32-make to build the library.  But first, you should
@ echo  look at Makefile and change the installation locations
@ echo.
