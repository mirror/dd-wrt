rem /* DOS 32 bit - Watcom C 10.5
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off
REM set WCGMEMORY=4096
REM set WCGMEMORY=?

set _CC=wcl386 -zq -mf -5r -bt#dos
set CFLAGS=-Iinclude -wx -oneatx
set MYLIB=lzo.lib

echo Compiling, please be patient...
set CC=%_CC%
REM set CC=%CC% -cc++
%CC% %CFLAGS% -DLZO_BUILD -zc -c src\*.c
@if errorlevel 1 goto error
%CC% -DLZO_BUILD -wx -c src\i386\d_asm2\*.asm
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
wlib -q -b -n -t %MYLIB% @b\dos32\wc105.rsp
@if errorlevel 1 goto error

set CC=%_CC% -zc -l#dos4g
%CC% %CFLAGS% ltest\ltest.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% examples\dict.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\lpack.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\overlap.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\precomp.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\precomp2.c %MYLIB%
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
