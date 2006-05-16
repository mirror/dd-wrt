rem /* DOS 32 bit - gcc 2.6.3 (djgpp v1)
rem  * a very simple make driver
rem  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
rem  */

@if "%LZO_ECHO%"=="n" echo off

set EXTRA_CFLAGS=

set CC=gcc
set CFLAGS=@b\gcc.opt @b\dos32\dj1.opt @b\gcc_lzo.opt %EXTRA_CFLAGS%
set ASFLAGS=-x assembler-with-cpp -Wall
set MYLIB=liblzo.a

echo Compiling, please be patient...
%CC% %CFLAGS% -DLZO_BUILD -Isrc -c src\*.c
@if errorlevel 1 goto error
%CC% %ASFLAGS% -DLZO_BUILD -Isrc/i386/src -c src\i386\src\*.s
@if errorlevel 1 goto error
if exist %MYLIB% del %MYLIB%
ar rcs %MYLIB% @b\dos32\dj1.rsp
@if errorlevel 1 goto error

set CFLAGS=@b\gcc.opt @b\dos32\dj1.opt %EXTRA_CFLAGS%
%CC% -s %CFLAGS% -Iltest ltest\ltest.c %MYLIB% -o ltest.out
@if errorlevel 1 goto error
coff2exe ltest.out

%CC% -s %CFLAGS% examples\dict.c %MYLIB% -o dict.out
@if errorlevel 1 goto error
coff2exe dict.out
%CC% -s %CFLAGS% examples\lpack.c %MYLIB% -o lpack.out
@if errorlevel 1 goto error
coff2exe lpack.out
%CC% -s %CFLAGS% examples\overlap.c %MYLIB% -o overlap.out
@if errorlevel 1 goto error
coff2exe overlap.out
%CC% -s %CFLAGS% examples\precomp.c %MYLIB% -o precomp.out
@if errorlevel 1 goto error
coff2exe precomp.out
%CC% -s %CFLAGS% examples\precomp2.c %MYLIB% -o precomp2.out
@if errorlevel 1 goto error
coff2exe precomp2.out
%CC% -s %CFLAGS% examples\simple.c %MYLIB% -o simple.out
@if errorlevel 1 goto error
coff2exe simple.out

%CC% -s %CFLAGS% -Isrc tests\align.c %MYLIB% -o align.out
@if errorlevel 1 goto error
coff2exe align.out
%CC% -s %CFLAGS% -Isrc tests\chksum.c %MYLIB% -o chksum.out
@if errorlevel 1 goto error
coff2exe chksum.out

set CFLAGS=@b\gcc.opt @b\dos32\dj1.opt @b\gcc_lzo.opt %EXTRA_CFLAGS%
%CC% -s %CFLAGS% -Iminilzo minilzo\*.c -o testmini.out
@if errorlevel 1 goto error
coff2exe testmini.out

@echo Done.
@goto end
:error
echo error!
:end
@call b\unset.bat
