#! /bin/sh

export AR='mipsel-linux-uclibc-ar'
export AS='mipsel-linux-uclibc-as'
export CC='mipsel-linux-uclibc-gcc'
export LD='mipsel-linux-uclibc-ld'
export NM='mipsel-linux-uclibc-nm'
export OBJCOPY='mipsel-linux-uclibc-objcopy'
export RANLIB='mipsel-linux-uclibc-ranlib'
export STRIP='mipsel-linux-uclibc-strip'
export LDFLAGS='-Wl,-static -static -static-libgcc -s -Wl,--gc-sections'
export CFLAGS='-Os -fomit-frame-pointer'

./configure --host=mipsel-linux && \
  make -j3
