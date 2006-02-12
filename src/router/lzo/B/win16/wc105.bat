rem /* Windows 16 bit (LIB) - Watcom C 10.5
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set _CC=wcl -zq -ml -5 -bt#windows
set CFLAGS=-Iinclude -wx -oneatx
REM set CFLAGS=%CFLAGS% -D__LZO_STRICT_16BIT
set MYLIB=lzo.lib

echo Compiling, please be patient...
set CC=%_CC%
%CC% %CFLAGS% -DLZO_BUILD -zc -c src\*.c
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
wlib -q -b -n -t %MYLIB% @b\win16\wc105.rsp
@if errorlevel 1 goto error

set CC=%_CC% -zc -bw -l#windows
%CC% %CFLAGS% ltest\ltest.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% examples\lpack.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\overlap.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\simple.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Isrc tests\align.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -Isrc tests\chksum.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -fe#testmini.exe minilzo\*.c
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
