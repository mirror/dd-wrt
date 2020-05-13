# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux user-space PPPoE client.
#
# Copyright (C) 2000 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# LIC: GPL
#
# $Id: Makefile,v 1.1.8.1 2004/08/01 13:08:03 boris Exp $
#***********************************************************************

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
include $(TOP)/.config



VERSION=3.11

DEFINES=
prefix=/usr
exec_prefix=${prefix}
mandir=${prefix}/man
docdir=/usr/doc/rp-pppoe-$(VERSION)
install=/usr/bin/install -c
install_dir=/usr/bin/install -c -d
sbindir=${exec_prefix}/sbin

# Plugin for pppd on Linux
LINUX_KERNELMODE_PLUGIN=rp-pppoe
PPPD_INCDIR=../../pppd.new

# Licensed version
LIC_INCDIR=
LIC_LIBDIR=
LIC_LIB=
LIC_DEFINE=

# PPPoE relay -- currently only supported on Linux
PPPOE_RELAY=pppoe-relay

# Program paths
PPPOE_PATH=$(sbindir)/pppoe
PPPD_PATH=$(sbindir)/pppd

# Include ServPoET version if we're building for ServPoET


# Kernel-mode plugin gets installed here.
PLUGIN_DIR=/usr/lib
PLUGIN_PATH=$(PLUGIN_DIR)/rp-pppoe.so

# Configuration file paths
PPPOESERVER_PPPD_OPTIONS=/tmp/pppoeserver/pppoe-server-options

PATHS='-DPPPOE_PATH="$(PPPOE_PATH)"' '-DPPPD_PATH="$(PPPD_PATH)"' \
	'-DPLUGIN_PATH="$(PLUGIN_PATH)"' \
	'-DPPPOE_SERVER_OPTIONS="$(PPPOESERVER_PPPD_OPTIONS)"'

CFLAGS= $(COPTS) $(MIPS16_OPT) -fno-strict-aliasing -fcommon -Wall -Wstrict-prototypes -ansi -pedantic $(LIC_INCDIR) $(DEFINES) $(LIC_DEFINE) $(PATHS) -Ilibevent -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(CONFIG_PPPOERELAY),y)
CFLAGS += -DHAVE_PPPOERELAY
endif
ifeq ($(CONFIG_PPPOESERVER),y)
CFLAGS += -DHAVE_PPPOESERVER
endif

TARGETS=pppoe-multi pppoe
PPPOE_SERVER_LIBS=$(LIC_LIBDIR) $(LIC_LIB)

all: $(TARGETS)
	@echo ""
	@echo "Type 'make install' as root to install the software."

pppoe-sniff: pppoe-sniff.o if.o common.o debug.o
	$(CC) -o pppoe-sniff pppoe-sniff.o if.o common.o debug.o

ifeq ($(CONFIG_PPPOESERVER),y)
pppoe-multi: multicall.o pppoe-server.o if.o debug.o common.o relay.o md5.o libevent/libevent.a 
	$(CC) -ffunction-sections -fdata-sections -Wl,--gc-sections -o pppoe-multi relay.o multicall.o pppoe-server.o if.o debug.o common.o md5.o $(PPPOE_SERVER_LIBS) -Llibevent -levent
else
pppoe-multi: multicall.o if.o debug.o common.o relay.o md5.o  
	$(CC) -ffunction-sections -fdata-sections -Wl,--gc-sections -o pppoe-multi relay.o multicall.o if.o debug.o common.o

endif
pppoe: pppoe.o if.o debug.o common.o ppp.o discovery.o
	$(CC) -o pppoe pppoe.o if.o debug.o common.o ppp.o discovery.o

pppoe-relay: relay.o if.o debug.o common.o
	$(CC) -o pppoe-relay relay.o if.o debug.o common.o

pppoe.o: pppoe.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe.o pppoe.c

discovery.o: discovery.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o discovery.o discovery.c

ppp.o: ppp.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o ppp.o ppp.c

md5.o: md5.c md5.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o md5.o md5.c

pppoe-server.o: pppoe-server.c pppoe.h 
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe-server.o pppoe-server.c

pppoe-sniff.o: pppoe-sniff.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe-sniff.o pppoe-sniff.c

if.o: if.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o if.o if.c

libevent/libevent.a:
	cd libevent && $(MAKE)

common.o: common.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o common.o common.c

debug.o: debug.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o debug.o debug.c

relay.o: relay.c relay.h pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o relay.o relay.c

# Linux-specific plugin
rp-pppoe.so: plugin/libplugin.a plugin/plugin.o
	$(CC) -o rp-pppoe.so -shared plugin/plugin.o plugin/libplugin.a

plugin/plugin.o: plugin.c
	$(CC) '-DRP_VERSION="$(VERSION)"' $(CFLAGS) -I$(PPPD_INCDIR) -c -o plugin/plugin.o -fPIC plugin.c

plugin/libplugin.a: plugin/discovery.o plugin/if.o plugin/common.o plugin/debug.o
	$(AR) -rc $@ $^

plugin/discovery.o: discovery.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o plugin/discovery.o -fPIC discovery.c

plugin/if.o: if.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o plugin/if.o -fPIC if.c

plugin/debug.o: debug.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o plugin/debug.o -fPIC debug.c

plugin/common.o: common.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o plugin/common.o -fPIC common.c

install: all
	-mkdir -p $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 -s pppoe $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 -s pppoe-server $(RPM_INSTALL_ROOT)$(sbindir)
	if test -x licensed-only/pppoe-server-control ; then $(install) -m 755 -s licensed-only/pppoe-server-control $(RPM_INSTALL_ROOT)$(sbindir); fi
	if test -x pppoe-relay ; then $(install) -m 755 -s pppoe-relay $(RPM_INSTALL_ROOT)$(sbindir); fi
	if test -x pppoe-sniff; then $(install) -m 755 -s pppoe-sniff $(RPM_INSTALL_ROOT)$(sbindir); fi
	$(install) -m 755 ../scripts/adsl-connect $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 ../scripts/adsl-start $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 ../scripts/adsl-status $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 ../scripts/adsl-stop $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 ../scripts/adsl-setup $(RPM_INSTALL_ROOT)$(sbindir)
	-mkdir -p $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../doc/CHANGES $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../doc/KERNEL-MODE-PPPOE $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../doc/HOW-TO-CONNECT $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../doc/LICENSE $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../README $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../SERVPOET $(RPM_INSTALL_ROOT)$(docdir)
	$(install) -m 644 ../configs/pap-secrets $(RPM_INSTALL_ROOT)$(docdir)
	-mkdir -p $(RPM_INSTALL_ROOT)$(mandir)/man8
	for i in $(TARGETS) ; do \
		if test -f ../man/$$i.8 ; then \
			$(install) -m 644 ../man/$$i.8 $(RPM_INSTALL_ROOT)$(mandir)/man8 || exit 1; \
		fi; \
	done
	$(install) -m 644 ../man/adsl-start.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	$(install) -m 644 ../man/adsl-stop.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	$(install) -m 644 ../man/adsl-status.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	$(install) -m 644 ../man/adsl-connect.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	$(install) -m 644 ../man/adsl-setup.8 $(RPM_INSTALL_ROOT)$(mandir)/man8
	-mkdir -p $(RPM_INSTALL_ROOT)$(mandir)/man5
	$(install) -m 644 ../man/pppoe.conf.5 $(RPM_INSTALL_ROOT)$(mandir)/man5
	-mkdir -p $(RPM_INSTALL_ROOT)/etc/ppp
	-mkdir -p $(RPM_INSTALL_ROOT)$(PLUGIN_DIR)
	-echo "# Directory created by rp-pppoe for kernel-mode plugin" > $(RPM_INSTALL_ROOT)$(PLUGIN_DIR)/README
	@if test -r rp-pppoe.so; then $(install) -m 755 rp-pppoe.so $(RPM_INSTALL_ROOT)$(PLUGIN_DIR); fi
	@for i in pppoe.conf firewall-standalone firewall-masq ; do \
		if [ ! -f $(RPM_INSTALL_ROOT)/etc/ppp/$$i ] ; then \
			$(install) -m 644 ../configs/$$i $(RPM_INSTALL_ROOT)/etc/ppp ; \
		else \
			echo "NOT overwriting existing $(RPM_INSTALL_ROOT)/etc/ppp/$$i" ;\
			$(install) -m 644 ../configs/$$i $(RPM_INSTALL_ROOT)/etc/ppp/$$i-$(VERSION) ;\
		fi ;\
	done
	@if [ ! -f $(RPM_INSTALL_ROOT)$(PPPOESERVER_PPPD_OPTIONS) ] ; then \
		$(install) -m 644 ../configs/pppoe-server-options $(RPM_INSTALL_ROOT)$(PPPOESERVER_PPPD_OPTIONS) ; \
	else \
		echo "NOT overwriting existing $(RPM_INSTALL_ROOT)$(PPPOESERVER_PPPD_OPTIONS)"; \
		$(install) -m 644 ../configs/pppoe-server-options $(RPM_INSTALL_ROOT)$(PPPOESERVER_PPPD_OPTIONS)-example ; \
	fi
	@if [ -f /etc/redhat-release ] ; then \
		echo "Looks like a Red Hat system; installing $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl" ; \
		mkdir -p $(RPM_INSTALL_ROOT)/etc/rc.d/init.d ;\
		$(install) -m 755 ../scripts/adsl-init $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl ; \
	fi
	@if [ -f /etc/turbolinux-release ] ; then \
		echo "Looks like a TurboLinux system; installing $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl" ; \
		mkdir -p $(RPM_INSTALL_ROOT)/etc/rc.d/init.d ;\
		$(install) -m 755 ../scripts/adsl-init-turbolinux $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl ; \
	fi
	@if [ -f /etc/SuSE-release ] ; then \
		echo "Looks like a SuSE Linux system; installing $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl" ; \
		mkdir -p $(RPM_INSTALL_ROOT)/etc/rc.d/init.d ;\
		$(install) -m 755 ../scripts/adsl-init-suse $(RPM_INSTALL_ROOT)/etc/rc.d/init.d/adsl ; \
	fi

	# L2TP
	@if [ -f l2tp/handlers/sync-pppd.so ] ; then \
		mkdir -p $(RPM_INSTALL_ROOT)/usr/lib/l2tp/plugins ; \
		$(install) -m 755 l2tp/handlers/sync-pppd.so $(RPM_INSTALL_ROOT)/usr/lib/l2tp/plugins ; \
		mkdir -p $(RPM_INSTALL_ROOT)/etc/l2tp ; \
		$(install) -m 600 l2tp/l2tp.conf $(RPM_INSTALL_ROOT)/etc/l2tp/l2tp.conf.example ; \
	fi
	@echo ""
	@echo "Type 'adsl-setup' to configure the software."

servpoet-tgz: distro-servpoet
	cd .. && tar cvf servpoet-$(VERSION)$(BETA).tar servpoet-$(VERSION)$(BETA)
	gzip -f -v -9 ../servpoet-$(VERSION)$(BETA).tar
	rm -rf ../servpoet-$(VERSION)$(BETA)

tgz: distro
	cd .. && tar cvf rp-pppoe-$(VERSION)$(BETA).tar rp-pppoe-$(VERSION)$(BETA)
	gzip -f -v -9 ../rp-pppoe-$(VERSION)$(BETA).tar
	rm -rf ../rp-pppoe-$(VERSION)$(BETA)

distro-servpoet: distro
	cp ../servpoet.spec ../rp-pppoe-$(VERSION)$(BETA)
	$(MAKE) -C licensed-only distro VERSION=$(VERSION) BETA=$(BETA)
	mv ../rp-pppoe-$(VERSION)$(BETA) ../servpoet-$(VERSION)$(BETA)

distro:
	rm -rf ../rp-pppoe-$(VERSION)$(BETA)
	mkdir ../rp-pppoe-$(VERSION)$(BETA)
	for i in README SERVPOET go go-gui rp-pppoe.spec ; do \
		cp ../$$i ../rp-pppoe-$(VERSION)$(BETA) || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/gui
	for i in en.msg ja.msg Makefile.in tkpppoe.in wrapper.c tkpppoe.1 pppoe-wrapper.1 ; do \
		cp ../gui/$$i ../rp-pppoe-$(VERSION)$(BETA)/gui || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/gui/html
	for i in mainwin-busy.png mainwin-nonroot.png mainwin.png props-advanced.png props-basic.png props-nic.png props-options.png tkpppoe.html ; do \
		cp ../gui/html/$$i ../rp-pppoe-$(VERSION)$(BETA)/gui/html || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/configs
	for i in firewall-masq firewall-standalone pap-secrets pppoe-server-options pppoe.conf ; do \
		cp ../configs/$$i ../rp-pppoe-$(VERSION)$(BETA)/configs || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/doc
	for i in CHANGES KERNEL-MODE-PPPOE HOW-TO-CONNECT LICENSE PROBLEMS ; do \
		cp ../doc/$$i ../rp-pppoe-$(VERSION)$(BETA)/doc || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/man
	for i in adsl-connect.8 adsl-setup.8 adsl-start.8 adsl-status.8 adsl-stop.8 pppoe-server.8 pppoe-sniff.8 pppoe.8 pppoe-relay.8 pppoe.conf.5 ; do \
		cp ../man/$$i ../rp-pppoe-$(VERSION)$(BETA)/man || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/scripts
	for i in adsl-connect.in adsl-init-suse.in adsl-init-turbolinux.in adsl-init.in adsl-setup.in adsl-start.in adsl-stop.in adsl-status ; do \
		cp ../scripts/$$i ../rp-pppoe-$(VERSION)$(BETA)/scripts || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/src
	for i in Makefile.in install-sh common.c config.h.in configure configure.in debug.c discovery.c if.c md5.c md5.h ppp.c pppoe-server.c pppoe-sniff.c pppoe.c pppoe.h pppoe-server.h plugin.c relay.c relay.h ; do \
		cp ../src/$$i ../rp-pppoe-$(VERSION)$(BETA)/src || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/src/libevent
	for i in Makefile.in event.c event.h event_tcp.c event_tcp.h eventpriv.h hash.c hash.h event_sig.c ; do \
		cp ../src/libevent/$$i ../rp-pppoe-$(VERSION)$(BETA)/src/libevent || exit 1; \
	done
	mkdir ../rp-pppoe-$(VERSION)$(BETA)/src/plugin

distro-beta: beta-check
	$(MAKE) distro BETA=-BETA-$(BETA)

beta-check:
	@if test "$(BETA)" = "" ; then \
		echo "Usage: make distro-beta BETA=<x>"; \
		exit 1; \
	fi

rpms: tgz
	cp ../rp-pppoe-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	gpg --detach-sign /usr/src/redhat/SOURCES/rp-pppoe-$(VERSION).tar.gz
	cd ..; \
	rpm -ba rp-pppoe.spec
	rpm --addsign /usr/src/redhat/SRPMS/rp-pppoe-$(VERSION)-1.src.rpm \
		/usr/src/redhat/RPMS/i386/rp-pppoe-$(VERSION)-1.i386.rpm \
		/usr/src/redhat/RPMS/i386/rp-pppoe-gui-$(VERSION)-1.i386.rpm

servpoet-rpms: servpoet-tgz
	cp ../servpoet-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	cd .. && rpm -ba servpoet.spec

clean:
	rm -f *.o pppoe pppoe-sniff pppoe-relay pppoe-server core rp-pppoe.so plugin/*.o plugin/libplugin.a *~
	test -f licensed-only/Makefile && $(MAKE) -C licensed-only clean || true
	test -f libevent/Makefile && $(MAKE) -C libevent clean || true
	test -f l2tp/Makefile && $(MAKE) -C l2tp clean || true

distclean: clean
	rm -f Makefile config.h config.cache config.log config.status
	rm -f libevent/Makefile
	rm -f ../scripts/adsl-connect ../scripts/adsl-start ../scripts/adsl-stop ../scripts/adsl-init ../scripts/adsl-setup ../scripts/adsl-init-suse ../scripts/adsl-init-turbolinux

update-version:
	sed -e 's/^Version: .*$$/Version: $(VERSION)/' ../rp-pppoe.spec > ../rp-pppoe.spec.new && mv ../rp-pppoe.spec.new ../rp-pppoe.spec
	sed -e 's+^Source: .*$$+Source: http://www.roaringpenguin.com/pppoe/rp-pppoe-$(VERSION).tar.gz+' ../rp-pppoe.spec > ../rp-pppoe.spec.new && mv ../rp-pppoe.spec.new ../rp-pppoe.spec
	test -f ../servpoet.spec && sed -e 's/^Version: .*$$/Version: $(VERSION)/' ../servpoet.spec > ../servpoet.spec.new && mv ../servpoet.spec.new ../servpoet.spec || true
	test -f ../servpoet.spec && sed -e 's+^Source: .*$$+Source: http://www.roaringpenguin.com/pppoe/servpoet-$(VERSION).tar.gz+' ../servpoet.spec > ../servpoet.spec.new && mv ../servpoet.spec.new ../servpoet.spec || true

# Convenience target for David!  Don't try to use this one.
km:
	./configure --enable-plugin=/home/dfs/Archive/PPP/ppp-2.4.0.pppoe4-patched-dfs

licensed-only:
	$(MAKE) -C licensed-only all VERSION=$(VERSION) SERVPOET_VERSION=$(SERVPOET_VERSION)

l2tp: libevent/libevent.a
	$(MAKE) -C l2tp all

.PHONY: update-version

.PHONY: clean

.PHONY: distclean

.PHONY: rpms

.PHONY: licensed-only

.PHONY: distro

.PHONY: l2tp
