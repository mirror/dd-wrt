rem /* Windows 32 bit (LIB) - Borland C/C++ 5.5.1
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set CC=bcc32
set CFLAGS=-Iinclude -O2 -d -w -w-aus
set MYLIB=lzo.lib

echo Compiling, please be patient...
%CC% %CFLAGS% -DLZO_BUILD -Isrc -c src\*.c
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
tlib %MYLIB% @b\win32\bc55.rsp
@if errorlevel 1 goto error

%CC% %CFLAGS% -Iltest ltest\ltest.c %MYLIB%
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

%CC% %CFLAGS% -Iminilzo -etestmini.exe minilzo\*.c
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
