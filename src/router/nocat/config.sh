#!/bin/sh

CC="mipsel-uclibc-gcc" CFLAGS="$(COPTS)" \
./configure --with-remote-splash --prefix=/usr --localstatedir=/var --sysconfdir=/etc \
--with-glib-prefix=${PWD}/../glib-1.2.10-install \
--disable-glibtest --host=mipsel-linux
