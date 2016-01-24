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

export CFLAGS="-DUSE_RESTRICTED_HEADERS -g3"
export CC="diet $CC"
rm -f config.cache
$c --disable-shared --enable-debug --enable-linker-script $@ && \
  make clean && make check
