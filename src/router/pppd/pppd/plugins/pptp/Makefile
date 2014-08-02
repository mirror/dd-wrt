# Makefile for RADIUS plugin
#
# Copyright 2002 Roaring Penguin Software Inc.
#

DESTDIR = /usr/local
MANDIR = $(DESTDIR)/share/man/man8
LIBDIR = $(DESTDIR)/lib/pppd/$(VERSION)

VERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../../patchlevel.h)

INSTALL	= install

PLUGIN=pptp
CFLAGS=-Wall -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include  -I. -I../../../ $(COPTS)  $(MIPS16_OPT) -fPIC -DRC_LOG_FACILITY=LOG_DAEMON -DNEED_PRINTF



all: pptp.so

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -c -m 755 pptp.so $(LIBDIR)


CLIENTOBJS = pptp.o pptp_callmgr.o pptp_ctrl.o util.o vector.o \
	dirutil.o pptp_quirks.o orckit_quirks.o

pptp.so: $(CLIENTOBJS)
	$(CC) -o pptp.so -shared $(CLIENTOBJS)



clean:
	rm -f *.o *.so *.a

distclean:
	rm -f *.o *.so *.a

dist-clean: distclean
