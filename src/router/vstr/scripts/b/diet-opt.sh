#! /bin/sh

if false; then
 echo "Not reached."
elif [ -f ./configure ]; then
        c=./configure
        D=Documentation/
elif [ -f ../configure ]; then
        c=../configure
        D=../Documentation/
else
  echo "Not in right place, dying."
  exit 1;
fi

# BEG:

if [ "x$CC" = "x" ]; then
 CC=gcc
fi

export expot CFLAGS="-DUSE_RESTRICTED_HEADERS -O2 -march=i386 -mcpu=i686"
export CC="diet $CC"

$c --disable-shared --enable-linker-script $@ && \
 make clean && make check
