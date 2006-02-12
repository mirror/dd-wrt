rem /* Windows 32 bit (LIB) - Cygnus GNU-Win32 Development Kit
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set CC=gcc -s
set CFLAGS=-Iinclude -O2 -fomit-frame-pointer -Wall -W -Wno-uninitialized
set ASFLAGS=-x assembler-with-cpp -Wall
set MYLIB=liblzo.a

del *.o
echo Compiling, please be patient...
%CC% -x c %CFLAGS% -DLZO_BUILD -c src/*.[cC]
@if errorlevel 1 goto error
%CC% %ASFLAGS% -DLZO_BUILD -c src/i386/src/*.[sS]
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
ar rcs %MYLIB% *.o
@if errorlevel 1 goto error

%CC% %CFLAGS% -o ltest.exe ltest/ltest.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -o dict.exe examples/dict.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -o lpack.exe examples/lpack.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -o overlap.exe examples/overlap.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -o precomp.exe examples/precomp.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -o precomp2.exe examples/precomp2.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -o simple.exe examples/simple.c %MYLIB%
@if errorlevel 1 goto error

%CC% %CFLAGS% -Isrc -o align.exe tests/align.c %MYLIB%
@if errorlevel 1 goto error
%CC% %CFLAGS% -Isrc -o chksum.exe tests/chksum.c %MYLIB%
@if errorlevel 1 goto error

%CC% -x c %CFLAGS% -o testmini.exe minilzo/*.[cC]
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
