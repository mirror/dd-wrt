rem /* DOS 16 bit - Digital Mars C/C++ 8.28
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set CC=sc -ml
set CFLAGS=-Iinclude -o -w-
REM set CFLAGS=%CFLAGS% -D__LZO_STRICT_16BIT
set MYLIB=lzo.lib

echo Compiling, please be patient...
for %%i in (src\*.c) do %CC% %CFLAGS% -DLZO_BUILD -c %%i
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
lib %MYLIB% /b /c /n /noi @b\dos16\dmc.rsp
@if errorlevel 1 goto error

%CC% %CFLAGS% -Iltest ltest\ltest.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Iexamples examples\simple.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Isrc tests\align.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -Isrc tests\chksum.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Iminilzo -otestmini.exe minilzo\testmini.c minilzo\minilzo.c
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
