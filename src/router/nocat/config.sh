#!/bin/sh

CC="mipsel-uclibc-gcc" CFLAGS="-pipe -Os" \
./configure --prefix=/tmp/splish \
--with-glib-prefix=${PWD}/../glib-1.2.10-install \
--disable-glibtest --host=mipsel-linux
