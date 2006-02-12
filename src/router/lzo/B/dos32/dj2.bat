rem /* DOS 32 bit - gcc (djgpp v2)
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set EXTRA_CFLAGS=

set CC=gcc
set CFLAGS=@b/gcc.opt @b/dos32/dj2.opt @b/gcc_lzo.opt %EXTRA_CFLAGS%
set ASFLAGS=-x assembler-with-cpp -Wall
set MYLIB=liblzo.a

echo Compiling, please be patient...
%CC% %CFLAGS% -DLZO_BUILD -c src/*.c
@if errorlevel 1 goto error
%CC% %ASFLAGS% -DLZO_BUILD -c src/i386/src/*.s
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
ar rcs %MYLIB% @b/dos32/dj2.rsp
@if errorlevel 1 goto error

set CFLAGS=@b/gcc.opt @b/dos32/dj2.opt %EXTRA_CFLAGS%
%CC% -s %CFLAGS% ltest/ltest.c %MYLIB% -o ltest.exe
@if errorlevel 1 goto error

%CC% -s %CFLAGS% examples/dict.c %MYLIB% -o dict.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% examples/lpack.c %MYLIB% -o lpack.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% examples/overlap.c %MYLIB% -o overlap.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% examples/precomp.c %MYLIB% -o precomp.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% examples/precomp2.c %MYLIB% -o precomp2.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% examples/simple.c %MYLIB% -o simple.exe
@if errorlevel 1 goto error

%CC% -s %CFLAGS% -Isrc tests/align.c %MYLIB% -o align.exe
@if errorlevel 1 goto error
%CC% -s %CFLAGS% -Isrc tests/chksum.c %MYLIB% -o chksum.exe
@if errorlevel 1 goto error

set CFLAGS=@b/gcc.opt @b/dos32/dj2.opt @b/gcc_lzo.opt %EXTRA_CFLAGS%
%CC% -s %CFLAGS% minilzo/*.c -o testmini.exe
@if errorlevel 1 goto error

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
