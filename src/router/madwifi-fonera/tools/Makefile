#	$FreeBSD: src/tools/tools/ath/Makefile,v 1.3 2003/12/07 21:38:44 sam Exp $
#
# Copyright (c) 2002-2005	Sam Leffler, Errno Consulting
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
DEPTH=	..

# USER_CROSS_COMPILE is a string that is prepended to all toolchain
# executables, such as gcc, ld, as, objcopy etc.  This is used for
# cross-compiling userspace binaries.  If not defined, CROSS_COMPILE
# is used.
USER_CROSS_COMPILE ?= $(CROSS_COMPILE)
STRIP ?= $(USER_CROSS_COMPILE)strip
CC = $(USER_CROSS_COMPILE)gcc

BINDIR ?= /usr/local/bin
MANDIR ?= /usr/local/man

#
# Path to the HAL source code.
#
ifeq ($(HAL),)
HAL=   $(DEPTH)/hal
endif

INCS=  -include $(DEPTH)/include/compat.h


ALL=	athstats 80211stats athkey athchans athctrl \
	athdebug 80211debug wlanconfig
	
ifdef DOMULTI
OBJS=	do_multi.o athstats.o 80211stats.o athkey.o athchans.o athctrl.o \
	athdebug.o 80211debug.o wlanconfig.o 
ALL=	${OBJS} madwifi_multi
endif

all:	$(ALL)

INCS+=	-I. -I$(HAL) -I$(DEPTH)
CFLAGS=	-g -O2 -Wall
ALL_CFLAGS= $(CFLAGS) $(INCS)
LDFLAGS=

all:	$(ALL)

athstats.o: athstats.c
	${CC} -c -o athstats.o -DDOMULTI=1 ${ALL_CFLAGS} -I../ath athstats.c
80211stats.o: 80211stats.c
	${CC} -c -o 80211stats.o -DDOMULTI=1 ${ALL_CFLAGS} 80211stats.c
athkey.o: athkey.c
	${CC} -c -o athkey.o -DDOMULTI=1 ${ALL_CFLAGS} athkey.c
athchans.o: athchans.c
	${CC} -c -o athchans.o -DDOMULTI=1 ${ALL_CFLAGS} athchans.c
athctrl.o: athctrl.c
	${CC} -c -o athctrl.o -DDOMULTI=1 ${ALL_CFLAGS} athctrl.c
athdebug.o: athdebug.c
	${CC} -c -o athdebug.o -DDOMULTI=1 ${ALL_CFLAGS} athdebug.c
wlanconfig.o: wlanconfig.c
	${CC} -c -o wlanconfig.o -DDOMULTI=1 ${ALL_CFLAGS} wlanconfig.c
80211debug.o: 80211debug.c
	${CC} -c -o 80211debug.o -DDOMULTI=1 ${ALL_CFLAGS} 80211debug.c
do_multi.o: do_multi.c
	${CC} -c -o do_multi.o -DDOMULTI=1 ${ALL_CFLAGS} do_multi.c
madwifi_multi:
	${CC} -o madwifi_multi ${LDFLAGS} ${OBJS}
	for i in athstats 80211stats athkey athchans athctrl athdebug wlanconfig 80211debug; do \
	ln -s -f madwifi_multi $$i; \
	done

athstats: athstats.c
	$(CC) -o athstats $(ALL_CFLAGS) -I../ath $(LDFLAGS) athstats.c
80211stats: 80211stats.c
	$(CC) -o 80211stats $(ALL_CFLAGS) $(LDFLAGS) 80211stats.c
athkey: athkey.c
	$(CC) -o athkey $(ALL_CFLAGS) $(LDFLAGS) athkey.c
athchans: athchans.c
	$(CC) -o athchans $(ALL_CFLAGS) $(LDFLAGS) athchans.c
athctrl: athctrl.c
	$(CC) -o athctrl $(ALL_CFLAGS) $(LDFLAGS) athctrl.c
athdebug: athdebug.c
	$(CC) -o athdebug $(ALL_CFLAGS) $(LDFLAGS) athdebug.c
wlanconfig: wlanconfig.c
	$(CC) -o wlanconfig $(ALL_CFLAGS) $(LDFLAGS) wlanconfig.c
80211debug: 80211debug.c
	$(CC) -o 80211debug $(ALL_CFLAGS) $(LDFLAGS) 80211debug.c

install: $(ALL)
	install -d $(DESTDIR)$(BINDIR)
	for i in $(ALL); do \
		install $$i $(DESTDIR)$(BINDIR)/$$i; \
		$(STRIP) $(DESTDIR)$(BINDIR)/$$i; \
	done
#	install -d $(DESTDIR)$(MANDIR)/man8
#	install -m 0644 man/*.8 $(DESTDIR)$(MANDIR)/man8

uninstall:
	for i in $(ALL); do \
		rm -f $(DESTDIR)$(BINDIR)/$$i; \
	done
	for i in $(ALL:=.8); do \
		rm -f $(DESTDIR)$(MANDIR)/man8/$$i; \
	done

clean:
	rm -f $(ALL) core a.out
