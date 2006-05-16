#! /bin/sh
set -e

# /* PalmOS - m68k-palmos-coff-gcc 2.7.2.2-kgpd-071097
#  * a very simple make driver
#  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#  */

CC="m68k-palmos-coff-gcc"
CFLAGS="-Iinclude -O2 -fomit-frame-pointer -Wall -W -Wno-uninitialized"
CFLAGS="$CFLAGS -DNO_STDIO_H"
# CFLAGS="$CFLAGS -DNO_STDLIB_H"
# CFLAGS="$CFLAGS -S"
# CFLAGS="$CFLAGS -DLZO_DEBUG"
MYLIB=liblzo.a

rm -f *.o $MYLIB
echo Compiling, please be patient...
$CC -x c $CFLAGS -DSIZEOF_SIZE_T=4 -c src/*.[cC]
m68k-palmos-coff-ar rcs $MYLIB *.o

echo "Done."
