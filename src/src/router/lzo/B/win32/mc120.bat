rem /* Windows 32 bit (LIB) - Microsoft 32-bit C/C++ Compiler 12.00
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set _CC=cl -nologo -MD
set CFLAGS=-Iinclude -O2 -GF -W3
set MYLIB=lzo.lib

echo Compiling, please be patient...
set CC=%_CC%
%CC% %CFLAGS% -DLZO_BUILD -c src\*.c
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
lib -nologo -out:%MYLIB% @b\win32\mc120.rsp
@if errorlevel 1 goto error

set CC=%_CC% -DLZO_NO_ASM
%CC% %CFLAGS% ltest\ltest.c %MYLIB% setargv.obj
@if errorlevel 1 goto error

%CC% %CFLAGS% examples\dict.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\lpack.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\overlap.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\precomp.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\precomp2.c %MYLIB% setargv.obj
@if errorlevel 1 goto error
%CC% %CFLAGS% examples\simple.c %MYLIB% setargv.obj
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
