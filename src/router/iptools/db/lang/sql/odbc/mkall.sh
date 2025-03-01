#!/bin/sh
#
# Build everything for Win32/Win64

rm -rf dist
mkdir dist
exec >mkall.log 2>&1
set -x
if test -r VERSION ; then
    VER32=$(tr -d '.' <VERSION)
    VER=$(cat VERSION)
else
    VER32="0"
    VER="0.0"
fi

if test $(arch) = "x86_64" ; then
    CC32="gcc -m32 -march=i386 -mtune=i386"
    SH32="linux32 sh"
else
    CC32=gcc
    SH32=sh
fi

case "$REL" in
    [yYTt1-9]*)
	MVER32=""
	;;
    *)
	MVER32="-$VER32"
	;;
esac

export MSVCRT=""

echo -n >/dev/tty "sqliteodbc_dl$MVER32.exe ... "
NO_SQLITE2=1 NO_TCCEXT=1 SQLITE_DLLS=2 CC=$CC32 $SH32 mingw-cross-build.sh
mv sqliteodbc.exe dist/sqliteodbc_dl$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc$MVER32.exe ..."
CC=$CC32 $SH32 mingw-cross-build.sh
mv sqliteodbc.exe dist/sqliteodbc$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc_w64_dl$MVER32.exe ..."
NO_SQLITE2=1 SQLITE_DLLS=2 sh mingw64-cross-build.sh
mv sqliteodbc_w64.exe dist/sqliteodbc_w64_dl$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc_w64$MVER32.exe ..."
sh mingw64-cross-build.sh
mv sqliteodbc_w64.exe dist/sqliteodbc_w64$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

export MSVCRT="100"
MVER32="_msvcr100$MVER32"

echo -n >/dev/tty "sqliteodbc_dl$MVER32.exe ..."
NO_SQLITE2=1 NO_TCCEXT=1 SQLITE_DLLS=2 CC=$CC32 $SH32 mingw-cross-build.sh
mv sqliteodbc.exe dist/sqliteodbc_dl$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc$MVER32.exe ..."
CC=$CC32 $SH32 mingw-cross-build.sh
mv sqliteodbc.exe dist/sqliteodbc$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc_w64_dl$MVER32.exe ..."
NO_SQLITE2=1 SQLITE_DLLS=2 sh mingw64-cross-build.sh
mv sqliteodbc_w64.exe dist/sqliteodbc_w64_dl$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc_w64$MVER32.exe ..."
sh mingw64-cross-build.sh
mv sqliteodbc_w64.exe dist/sqliteodbc_w64$MVER32.exe
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

echo -n >/dev/tty "sqliteodbc-$VER.tar.gz ..."
test -r ../sqliteodbc-$VER.tar.gz && cp -p ../sqliteodbc-$VER.tar.gz dist
if test $? = 0 ; then echo >/dev/tty OK ; else echo >/dev/tty ERROR ; fi

