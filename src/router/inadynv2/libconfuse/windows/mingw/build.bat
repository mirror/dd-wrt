@echo off
rem
rem Compilation script of the static library for MinGW.
rem
echo.
echo Compiling libconfuse.a for MinGW...
echo.

rem Source directory
set SRC_DIR=..\..\src

set INCLUDE=-I. -I%SRC_DIR%
set CFLAGS=-Wall -DHAVE_CONFIG_H -DHAVE_STRCASECMP %INCLUDE% -c

set COMPILE=gcc %CFLAGS% %SRC_DIR%\confuse.c
echo  %COMPILE%
%COMPILE% || goto fatal

set COMPILE=gcc %CFLAGS% %SRC_DIR%\lexer.c
echo  %COMPILE%
%COMPILE% || goto fatal

set LINK=ar rc libconfuse.a confuse.o lexer.o
echo  %LINK%
%LINK% || goto fatal

del confuse.o
del lexer.o

echo.
echo  [ OK ]
echo.
goto end

:fatal
echo.
echo  [ FAILED ]
echo.

:end
pause
