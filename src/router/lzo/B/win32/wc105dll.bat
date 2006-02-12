rem /* Windows 32 bit (DLL) - Watcom C 10.5
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set _CC=wcl386 -zq -mf -5r -bt#nt
set CFLAGS=-Iinclude -wx -oneatx
set MYDLL=lzo.dll
set MYLIB=lzo.lib

echo Compiling, please be patient...
set CC=%_CC%
%CC% %CFLAGS% -DLZO_BUILD_DLL -zc -bd -D__LZO_EXPORT2=__export -c src\*.c
@if errorlevel 1 goto error
set CC=%_CC%
%CC% -wx -c src\i386\d_asm2\*.asm
@if errorlevel 1 goto error
if exist %MYDLL% del %MYDLL%
if exist %MYLIB% del %MYLIB%
wlink name %MYDLL% @b\win32\wc105dll.def @b\win32\wc105dll.rsp
@if errorlevel 1 goto error
wlib -q -b -n -t %MYLIB% +%MYDLL%
@if errorlevel 1 goto error

set CC=%_CC% -zc -l#nt
set CFLAGS=%CFLAGS% -DLZO_USE_DLL -DLZO_NO_ASM

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
