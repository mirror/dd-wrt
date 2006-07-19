#***********************************************************************
#
# Makefile
#
# Makefile for L2TP code.
#
# Copyright (C) 2002 Roaring Penguin Software Inc.
#
# This software may be distributed under the terms of the GNU General
# Public License, Version 2, or (at your option) any later version.
#
# LIC: GPL
#
# $Id: Makefile,v 1.1 2004/02/04 04:01:51 kanki Exp $
#***********************************************************************

VERSION=0.3

DEFINES=
prefix=/usr
exec_prefix=${prefix}
mandir=${prefix}/man
docdir=/usr/doc/l2tp-$(VERSION)
install=/usr/bin/install -c
install_dir=/usr/bin/install -c -d
sbindir=${exec_prefix}/sbin

OBJS=auth.o debug.o dgram.o main.o md5.o network.o options.o peer.o session.o tunnel.o utils.o
EXTRA_LIBS=libevent/*.o -ldl

SRCS=$(OBJS:.o=.c)
CFLAGS=$(COPTS) -I.. -Ilibevent -Wall -Wstrict-prototypes -D_GNU_SOURCE -DVERSION=\"$(VERSION)\" -DPREFIX=\"$(prefix)\"

TARGETS=l2tpd libl2tp.a handlers

all: $(TARGETS)

libl2tp.a: $(OBJS)
	rm -f $@
	ar -rc $@ $^
	ranlib $@ || true

l2tpd: libl2tp.a libevent/libevent.a
	$(MAKE) -C handlers
	$(CC) -o l2tpd -rdynamic $(OBJS) $(EXTRA_LIBS)

libevent/libevent.a:
	test -d libevent || ln -s ../libevent .
	$(MAKE) -C libevent

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

depend: .depend

.depend: $(SRCS)
	$(CC) -M $(CFLAGS) $^ > .depend

handlers:
	$(MAKE) -C handlers

# Release requires GNU tar!!
release: FORCE
	rm -f rp-l2tp-$(VERSION).tar.gz
	make -C libevent/Doc
	./make-release.sh $(VERSION)
	tar cvhf rp-l2tp-$(VERSION).tar rp-l2tp-$(VERSION)
	gzip -v -9 rp-l2tp-$(VERSION).tar

clean:
	rm -f *.o *.a *~
	rm -f l2tpd
	rm -f libevent/*.o libevent/*.a
	$(MAKE) -C handlers clean

distclean: clean
	rm -f Makefile config.cache
	$(MAKE) -C handlers distclean

-include .depend

install: all
	-mkdir -p $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 -s l2tpd $(RPM_INSTALL_ROOT)$(sbindir)
	-mkdir -p $(RPM_INSTALL_ROOT)/etc/l2tp
	$(install) -m 644 l2tp.conf $(RPM_INSTALL_ROOT)/etc/l2tp/l2tp.conf.example
	-mkdir -p $(RPM_INSTALL_ROOT)$(mandir)/man8
	-mkdir -p $(RPM_INSTALL_ROOT)$(mandir)/man5
	$(install) -m 644 man/l2tpd.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	$(install) -m 644 man/l2tp.conf.5 $(RPM_INSTALL_ROOT)$(mandir)/man5

	$(MAKE) -C handlers install

FORCE:

.PHONY: handlers

.PHONY: FORCE
