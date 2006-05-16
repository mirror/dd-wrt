rem /* Windows 16 bit (LIB) - Microsoft C 7.0
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set CC=cl -nologo -f- -AL -Mq
set CFLAGS=-Iinclude -O -G2 -W3
REM set CFLAGS=%CFLAGS% -D__LZO_STRICT_16BIT
set MYLIB=lzo.lib

echo Compiling, please be patient...
%CC% %CFLAGS% -DLZO_BUILD -c src\*.c
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
lib /nologo %MYLIB% @b\win16\mc70.rsp;
@if errorlevel 1 goto error

%CC% %CFLAGS% ltest\ltest.c %MYLIB% setargv.obj
@if errorlevel 1 goto error

%CC% %CFLAGS% examples\lpack.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\overlap.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\simple.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Isrc tests\align.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -Isrc tests\chksum.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Fetestmini.exe minilzo\*.c
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
