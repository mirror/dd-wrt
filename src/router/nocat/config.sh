#!/bin/sh

CC="mipsel-linux-uclibc-gcc" 
CFLAGS="-Os -pipe -mips32 -mtune=mips32 -funit-at-a-time -I../libghttp" \
./configure --with-remote-splash --prefix=/usr --localstatedir=/var --sysconfdir=/etc \
--with-glib-prefix=${PWD}/../glib-1.2.10-install \
--disable-glibtest --host=mipsel-linux
