# Makefile for an IPSec VPN client compatible with Cisco equipment.
# Copyright (C) 2002  Geoffrey Keating
# Copyright (C) 2003-2004  Maurice Massar
# Copyright (C) 2006-2007 Dan Villiom Podlaski Christiansen

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# $Id: Makefile 236 2007-09-05 20:40:59Z Joerg Mayer $
include $(TOP)/.config

DESTDIR=$(INSTALLDIR)
PREFIX=/usr
ETCDIR=/etc/vpnc
BINDIR=$(PREFIX)/bin
SBINDIR=$(PREFIX)/sbin
MANDIR=$(PREFIX)/share/man

SRCS = sysdep.c vpnc-debug.c isakmp-pkt.c tunip.c config.c dh.c math_group.c supp.c decrypt-utils.c
BINS = vpnc cisco-decrypt
OBJS = $(addsuffix .o,$(basename $(SRCS)))
BINOBJS = $(addsuffix .o,$(BINS))
BINSRCS = $(addsuffix .c,$(BINS))
VERSION := $(shell sh mk-version)
RELEASE_VERSION := $(shell cat VERSION)

# The license of vpnc (Gpl >= 2) is quite likely incompatible with the
# openssl license. Openssl is currently used to provide certificate
# support for vpnc (hybrid only).
# While it is OK for users to build their own binaries linking in openssl
# with vpnc and even providing dynamically linked binaries it is probably
# not OK to provide the binaries inside a distribution.
# See http://www.gnome.org/~markmc/openssl-and-the-gpl.html for further
# details.
# Some distributions like Suse and Fedora seem to think otherwise.

# Comment this in to obtain a binary with certificate support which is
# GPL incompliant though.
#OPENSSL_GPL_VIOLATION = -DOPENSSL_GPL_VIOLATION
#OPENSSLLIBS = -lcrypto

ifneq ($(CONFIG_WOLFSSL),y)
CFLAGS += -W -Wall $(LTO) -fPIC $(COPTS) -Wmissing-declarations -Wwrite-strings -I$(INSTALLDIR)/vpnc/usr/include -I$(SSLPATH)/include 
else
CFLAGS += -W -Wall $(LTO) -fPIC  $(COPTS) -Wmissing-declarations -Wwrite-strings -I$(INSTALLDIR)/vpnc/usr/include -DWOLFSSL_USE_OPTIONS_H -I$(TOP)/wolfssl/standard -I$(TOP)/wolfssl/ -I$(TOP)/wolfssl/wolfssl 
endif
CPPFLAGS = -DVERSION=\"$(VERSION)\" $(OPENSSL_GPL_VIOLATION)
LDFLAGS = $(LDLTO) -L$(INSTALLDIR)/vpnc/usr/lib -pthread
CFLAGS +=  -L$(INSTALLDIR)/vpnc/usr/lib 

ifeq ($(shell uname -s), SunOS)
LDFLAGS += -lnsl -lresolv -lsocket
endif
ifneq (,$(findstring Apple,$(shell $(CC) --version)))
# enabled in FSF GCC, disabled by default in Apple GCC
CFLAGS += -fstrict-aliasing -freorder-blocks -fsched-interblock
endif

all : gnupg $(BINS)

configure:
	rm -f libgpg-error/Makefile
	rm -f libgcrypt/Makefile
gnupg :
	rm -f vpnc
	if test ! -e "libgpg-error/Makefile"; then cd libgpg-error && ./configure --host=$(ARCH)-linux --prefix=/usr --disable-shared --enable-static  --libdir=/usr/lib --disable-nls AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"; fi
	make -C libgpg-error
	make -C libgpg-error install DESTDIR=$(INSTALLDIR)/vpnc
	if test ! -e "libgcrypt/Makefile"; then cd libgcrypt && CFLAGS="$(COPTS) -std=gnu89 -I$(INSTALLDIR)/vpnc/usr/include -I$(SSLPATH)/include" && ./configure --host=$(ARCH)-linux --enable-static --disable-shared --prefix=/usr --libdir=/usr/lib --with-gpg-error-prefix=$(INSTALLDIR)/vpnc/usr AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"; fi 

	make -C libgcrypt install DESTDIR=$(INSTALLDIR)/vpnc


vpnc : $(OBJS) vpnc.o
ifneq ($(CONFIG_WOLFSSL),y)
	$(CC) -o $@ $^ $(LDFLAGS) $(INSTALLDIR)/vpnc/usr/lib/libgcrypt.a  $(INSTALLDIR)/vpnc/usr/lib/libgpg-error.a -L$(SSLPATH) -lcrypto
	cp vpnc vpnc.openssl
else
	$(CC) -o $@ $^ $(LDFLAGS) $(INSTALLDIR)/vpnc/usr/lib/libgcrypt.a  $(INSTALLDIR)/vpnc/usr/lib/libgpg-error.a -L$(TOP)/wolfssl/standard/src/.libs -lwolfssl
	cp vpnc vpnc.wolfssl
endif

vpnc.8 : vpnc.8.template makeman.pl vpnc
	./makeman.pl

cisco-decrypt : cisco-decrypt.o  decrypt-utils.o
	$(CC) -o $@ $^ $(LDFLAGS) $(INSTALLDIR)/vpnc/usr/lib/libgcrypt.a  $(INSTALLDIR)/vpnc/usr/lib/libgpg-error.a

.depend: $(SRCS) $(BINSRCS)
	$(CC) -MM $(SRCS) $(BINSRCS) $(CFLAGS) $(CPPFLAGS) > $@

vpnc-debug.c vpnc-debug.h : isakmp.h enum2debug.pl
	perl -w ./enum2debug.pl isakmp.h >vpnc-debug.c 2>vpnc-debug.h

vpnc.ps : vpnc.c
	enscript -E -G -T 4 --word-wrap -o- $^ | psnup -2 /dev/stdin $@

../vpnc-%.tar.gz : vpnc-$*.tar.gz

etags :
	etags *.[ch]
ctags :
	ctags *.[ch]

vpnc-%.tar.gz :
	mkdir vpnc-$*
	svn info -R | awk -v RS='\n\n' -v FS='\n' '/Node Kind: file/ {print substr($$1,7)}' | \
		tar cT - | tar xC vpnc-$*/
	tar zcf ../$@ vpnc-$*
	rm -rf vpnc-$*

dist : VERSION vpnc.8 vpnc-$(RELEASE_VERSION).tar.gz

clean :
	-make -C libgcrypt clean
	-make -C libgpg-error clean
	-rm -f libgpg-error/Makefile
	-rm -f libgcrypt/Makefile
	-rm -f .depend
	-rm -f $(OBJS) $(BINOBJS) $(BINS) tags

distclean : clean
	-rm -f vpnc-debug.c vpnc-debug.h vpnc.ps vpnc.8 .depend

install-common: 
	install -d $(DESTDIR)$(ETCDIR) $(DESTDIR)$(BINDIR) $(DESTDIR)$(SBINDIR) 
	if [ "`uname -s | cut -c-6`" = "CYGWIN" ]; then \
		install vpnc-script-win $(DESTDIR)$(ETCDIR)/vpnc-script; \
		install vpnc-script-win.js $(DESTDIR)$(ETCDIR); \
	else \
		install vpnc-script $(DESTDIR)$(ETCDIR); \
	fi
	install -m600 vpnc.conf $(DESTDIR)$(ETCDIR)/default.conf
	install -m755 vpnc-disconnect $(DESTDIR)$(SBINDIR)
	install -m755 pcf2vpnc $(DESTDIR)$(BINDIR)

install : install-common
ifneq ($(CONFIG_WOLFSSL),y)
	install -D -m755 vpnc.openssl $(DESTDIR)$(SBINDIR)/vpnc
else
	install -D -m755 vpnc.wolfssl $(DESTDIR)$(SBINDIR)/vpnc
endif	
	-rm -rf $(DESTDIR)/usr/share
	-rm -rf $(DESTDIR)/usr/include
	-rm -rf $(DESTDIR)/usr/lib
	-rm -f $(DESTDIR)/usr/bin/gpg-error*
	-rm -f $(DESTDIR)/usr/bin/libgcrypt*

install-strip : install-common
	install -s -m755 vpnc $(DESTDIR)$(SBINDIR)

uninstall :
	rm -f $(DESTDIR)$(SBINDIR)/vpnc \
		$(DESTDIR)$(SBINDIR)/vpnc-disconnect \
		$(DESTDIR)$(BINDIR)/pcf2vpnc 
	@echo NOTE: remove $(DESTDIR)$(ETCDIR) manually

.PHONY : clean distclean dist all install install-strip uninstall

#
-include .depend
