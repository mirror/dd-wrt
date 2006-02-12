# Makefile for busybox
#
# Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#--------------------------------------------------------------
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------
noconfig_targets := menuconfig config oldconfig randconfig \
	defconfig allyesconfig allnoconfig clean distclean \
	release tags

ifndef TOPDIR
TOPDIR=$(CURDIR)/
endif
ifndef top_srcdir
top_srcdir=$(CURDIR)
endif
ifndef top_builddir
top_builddir=$(CURDIR)
endif

export srctree=$(top_srcdir)
vpath %/Config.in $(srctree)

include $(top_builddir)/Rules.mak
include $(top_builddir)/../.config
CONFIG_UDHCPD=n
CONFIG_UDHCPC=n

DIRS:=applets archival archival/libunarchive coreutils console-tools \
	debianutils editors findutils init miscutils modutils networking \
	networking/libiproute networking/udhcp procps loginutils shell \
	sysklogd util-linux libpwdgrp coreutils/libcoreutils libbb

SRC_DIRS:=$(patsubst %,$(top_srcdir)/%,$(DIRS))

ifeq ($(strip $(CONFIG_SELINUX)),y)
CFLAGS += -I/usr/include/selinux
LIBRARIES += -lsecure
endif
CFLAGS += -I../../include.v23

LDFLAGS	+= -L../nvram -L../mipsel-uclibc/install/nvram/usr/lib -L../shared -L../mipsel-uclibc/install/shared/usr/lib -lshared -L../libnet/lib  
LIBRARIES += ../rc/rc.a ../dnsmasq/src/dnsmasq.a ../httpd/httpd.a -lnvram -lnet -lcrypt


ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif

ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif
ifeq ($(CONFIG_IPROUTE2),y)
CFLAGS += -DHAVE_IPROUTE2
LIBRARIES += ../iproute2/ip/ip.a ../iproute2/lib/libnetlink.a ../iproute2/lib/libutil.a ../iproute2/tc/q_htb.o ../iproute2/tc/tc.o ../iproute2/tc/tc_qdisc.o ../iproute2/tc/tc_class.o ../iproute2/tc/tc_filter.o ../iproute2/tc/tc_util.o ../iproute2/tc/m_police.o ../iproute2/tc/m_estimator.o ../iproute2/tc/m_action.o ../iproute2/tc/q_fifo.o ../iproute2/tc/q_sfq.o ../iproute2/tc/q_red.o ../iproute2/tc/q_prio.o ../iproute2/tc/q_tbf.o ../iproute2/tc/q_cbq.o ../iproute2/tc/q_wrr.o ../iproute2/tc/f_rsvp.o ../iproute2/tc/f_u32.o ../iproute2/tc/f_route.o ../iproute2/tc/f_fw.o ../iproute2/tc/q_dsmark.o ../iproute2/tc/q_gred.o ../iproute2/tc/f_tcindex.o ../iproute2/tc/q_ingress.o ../iproute2/tc/q_hfsc.o ../iproute2/tc/m_gact.o ../iproute2/tc/m_mirred.o ../iproute2/tc/m_ipt.o ../iproute2/tc/m_pedit.o ../iproute2/tc/p_ip.o ../iproute2/tc/p_icmp.o ../iproute2/tc/p_tcp.o ../iproute2/tc/p_udp.o ../iproute2/tc/tc_core.o ../iproute2/tc/tc_red.o ../iproute2/tc/tc_cbq.o ../iproute2/tc/tc_estimator.o -ldl -lm
endif
ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS += -DHAVE_NEWMEDIA
endif
ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
endif

ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_EBTABLES),y)
CFLAGS += -DHAVE_EBTABLES
endif

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
LIBRARIES += ../rflow/rflow.a -lpthread
endif

#CFLAGS += -DHAVE_DDNS
#LIBRARIES += ../ipupdate/ez-ipupdate.a


ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
LIBRARIES += ../upnp/upnp.a
LIBRARIES += ../upnp/libupnp.a
LIBRARIES += -L../netconf -lnetconf
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_TELNET),y)
CFLAGS += -DHAVE_TELNET
endif


ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

ifeq ($(CONFIG_PPTPD),y)
#OBJS += pptpd.o
CFLAGS += -DHAVE_PPTPD
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
#OBJS += sshd.o
CFLAGS += -DHAVE_SSHD
LIBRARIES += -L../zlib ../dropbear/dropbear.a ../dropbear/libtomcrypt/libtomcrypt.a ../dropbear/libtommath/libtommath.a -lutil -lz  -lcrypt
#LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFWD
LIBRARIES += ../dhcpforwarder/dhcpfwd.a
endif
ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
LIBRARIES += ../pppd/pppd/pppd.a
endif
LIBRARIES += ../net-tools/arp.a ../net-tools/lib/libnet-tools.a		


ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
LIBRARIES += ../bird/obj/nest/all.o 
LIBRARIES += ../bird/obj/filter/all.o 
LIBRARIES += ../bird/obj/proto/bgp/all.o 
LIBRARIES += ../bird/obj/proto/ospf/all.o 
LIBRARIES += ../bird/obj/proto/pipe/all.o 
LIBRARIES += ../bird/obj/proto/rip/all.o 
LIBRARIES += ../bird/obj/proto/static/all.o 
LIBRARIES += ../bird/obj/conf/all.o
LIBRARIES += ../bird/obj/lib/birdlib.a

endif

ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
endif

ifeq ($(CONFIG_ZEBRA),y)
CFLAGS += -DHAVE_ZEBRA
endif

ifeq ($(CONFIG_WSHAPER),y)
CFLAGS += -DHAVE_WSHAPER
endif

ifeq ($(CONFIG_SVQOS),y)
CFLAGS += -DHAVE_SVQOS
endif

ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
#OBJS += snmp.o
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
#OBJS += ../rc/wol.o
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS += -DHAVE_NOCAT
endif

ifeq ($(CONFIG_SER),y)
CFLAGS += -DHAVE_SER
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DHAVE_FREEBIRD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFORWARD
endif

ifeq ($(CONFIG_DHCPRELAY),y)
CFLAGS += -DHAVE_DHCPRELAY
endif

#ifeq ($(CONFIG_OPENSSL),y)
#CFLAGS += -DHAVE_HTTPS
#endif

ifeq ($(CONFIG_MATRIXSSL),y)
#CFLAGS += -DHAVE_HTTPS
LIBRARIES += -L../matrixssl/src -lmatrixsslstatic -lpthread
endif

LIBRARIES += ../iptables/iptables.a ../iptables/extensions/libext.a ../iptables/libiptc/libiptc.a


CONFIG_CONFIG_IN = $(top_srcdir)/sysdeps/$(TARGET_OS)/Config.in
CONFIG_DEFCONFIG = $(top_srcdir)/sysdeps/$(TARGET_OS)/defconfig

ALL_DIRS:= $(DIRS) scripts/config
ALL_MAKEFILES:=$(patsubst %,%/Makefile,$(ALL_DIRS))

ifeq ($(KBUILD_SRC),)

ifdef O
  ifeq ("$(origin O)", "command line")
    KBUILD_OUTPUT := $(O)
  endif
endif

# That's our default target when none is given on the command line
.PHONY: _all
_all:

ifneq ($(KBUILD_OUTPUT),)
# Invoke a second make in the output directory, passing relevant variables
# check that the output directory actually exists
saved-output := $(KBUILD_OUTPUT)
KBUILD_OUTPUT := $(shell cd $(KBUILD_OUTPUT) && /bin/pwd)
$(if $(wildcard $(KBUILD_OUTPUT)),, \
     $(error output directory "$(saved-output)" does not exist))

.PHONY: $(MAKECMDGOALS)

$(filter-out _all,$(MAKECMDGOALS)) _all: $(KBUILD_OUTPUT)/Rules.mak $(KBUILD_OUTPUT)/Makefile
	$(MAKE) -C $(KBUILD_OUTPUT) \
	top_srcdir=$(CURDIR) \
	top_builddir=$(KBUILD_OUTPUT) \
	TOPDIR=$(KBUILD_OUTPUT)	\
	KBUILD_SRC=$(CURDIR) \
	-f $(CURDIR)/Makefile $@

$(KBUILD_OUTPUT)/Rules.mak:
	@echo > $@
	@echo top_srcdir=$(CURDIR) >> $@
	@echo top_builddir=$(KBUILD_OUTPUT) >> $@
	@echo include $(top_srcdir)/Rules.mak >> $@

$(KBUILD_OUTPUT)/Makefile:
	@echo > $@
	@echo top_srcdir=$(CURDIR) >> $@
	@echo top_builddir=$(KBUILD_OUTPUT) >> $@
	@echo KBUILD_SRC='$$(top_srcdir)' >> $@
	@echo include '$$(KBUILD_SRC)'/Makefile >> $@

# Leave processing to above invocation of make
skip-makefile := 1
endif # ifneq ($(KBUILD_OUTPUT),)
endif # ifeq ($(KBUILD_SRC),)

ifeq ($(skip-makefile),)

_all: all

ifeq ($(strip $(HAVE_DOT_CONFIG)),y)

all: busybox busybox.links doc

all_tree:	$(ALL_MAKEFILES)

$(ALL_MAKEFILES): %/Makefile: $(top_srcdir)/%/Makefile
	d=`dirname $@`; [ -d "$$d" ] || mkdir -p "$$d"; cp $< $@

# In this section, we need .config
-include $(top_builddir)/.config.cmd
include $(patsubst %,%/Makefile.in, $(SRC_DIRS))
-include $(top_builddir)/.depend

busybox: $(ALL_MAKEFILES) .depend include/config.h $(libraries-y)
	$(CC) $(LDFLAGS) -export-dynamic -o $@ -Wl,--start-group $(libraries-y) $(LIBRARIES) -Wl,--end-group
	$(STRIPCMD) $@

busybox.links: $(top_srcdir)/applets/busybox.mkll include/config.h $(top_srcdir)/include/applets.h
	- $(SHELL) $^ >$@

install: applets/install.sh busybox busybox.links
	$(SHELL) $< $(PREFIX)
ifeq ($(strip $(CONFIG_FEATURE_SUID)),y)
	@echo
	@echo
	@echo --------------------------------------------------
	@echo You will probably need to make your busybox binary
	@echo setuid root to ensure all configured applets will
	@echo work properly.
	@echo --------------------------------------------------
	@echo
endif

uninstall: busybox.links
	rm -f $(PREFIX)/bin/busybox
	for i in `cat busybox.links` ; do rm -f $(PREFIX)$$i; done

install-hardlinks: $(top_srcdir)/applets/install.sh busybox busybox.links
	$(SHELL) $< $(PREFIX) --hardlinks

check: busybox
	bindir=$(top_builddir) srcdir=$(top_srcdir)/testsuite \
	$(top_srcdir)/testsuite/runtest

# Documentation Targets
doc: docs/busybox.pod docs/BusyBox.txt docs/BusyBox.1 docs/BusyBox.html

docs/busybox.pod : $(top_srcdir)/docs/busybox_header.pod $(top_srcdir)/include/usage.h $(top_srcdir)/docs/busybox_footer.pod
	-mkdir -p docs
	- ( cat $(top_srcdir)/docs/busybox_header.pod; \
	    $(top_srcdir)/docs/autodocifier.pl $(top_srcdir)/include/usage.h; \
	    cat $(top_srcdir)/docs/busybox_footer.pod ) > docs/busybox.pod

docs/BusyBox.txt: docs/busybox.pod
	$(SECHO)
	$(SECHO) BusyBox Documentation
	$(SECHO)
	-mkdir -p docs
	-pod2text $< > $@

docs/BusyBox.1: docs/busybox.pod
	- mkdir -p docs
	- pod2man --center=BusyBox --release="version $(VERSION)" \
		$< > $@

docs/BusyBox.html: docs/busybox.net/BusyBox.html
	- mkdir -p docs
	-@ rm -f docs/BusyBox.html
	-@ cp docs/busybox.net/BusyBox.html docs/BusyBox.html

docs/busybox.net/BusyBox.html: docs/busybox.pod
	-@ mkdir -p docs/busybox.net
	-  pod2html --noindex $< > \
	    docs/busybox.net/BusyBox.html
	-@ rm -f pod2htm*

# The nifty new buildsystem stuff
scripts/mkdep: $(top_srcdir)/scripts/mkdep.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

scripts/split-include: $(top_srcdir)/scripts/split-include.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

.depend: scripts/mkdep include/config.h
	rm -f .depend .hdepend;
	mkdir -p include/config;
	scripts/mkdep -I include -- \
	  `find $(top_srcdir) -name \*.c -print | sed -e "s,^./,,"` >> .depend;
	scripts/mkdep -I include -- \
	  `find $(top_srcdir) -name \*.h -print | sed -e "s,^./,,"` >> .hdepend;

depend dep: .depend

include/config/MARKER: depend scripts/split-include
	scripts/split-include include/config.h include/config
	@ touch include/config/MARKER

include/config.h: .config
	@if [ ! -x $(top_builddir)/scripts/config/conf ] ; then \
	    $(MAKE) -C scripts/config conf; \
	fi;
	@$(top_builddir)/scripts/config/conf -o $(CONFIG_CONFIG_IN)

finished2:
	$(SECHO)
	$(SECHO) Finished installing...
	$(SECHO)

else # ifeq ($(strip $(HAVE_DOT_CONFIG)),y)

all: menuconfig

# configuration
# ---------------------------------------------------------------------------

$(ALL_MAKEFILES): %/Makefile: $(top_srcdir)/%/Makefile
	d=`dirname $@`; [ -d "$$d" ] || mkdir -p "$$d"; cp $< $@

scripts/config/conf: scripts/config/Makefile Rules.mak
	$(MAKE) -C scripts/config conf
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi

scripts/config/mconf: scripts/config/Makefile Rules.mak
	$(MAKE) -C scripts/config ncurses conf mconf
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi

menuconfig: scripts/config/mconf
	@./scripts/config/mconf $(CONFIG_CONFIG_IN)

config: scripts/config/conf
	@./scripts/config/conf $(CONFIG_CONFIG_IN)

oldconfig: scripts/config/conf
	@./scripts/config/conf -o $(CONFIG_CONFIG_IN)

randconfig: scripts/config/conf
	@./scripts/config/conf -r $(CONFIG_CONFIG_IN)

allyesconfig: scripts/config/conf
	@./scripts/config/conf -y $(CONFIG_CONFIG_IN)
	sed -i -e "s/^CONFIG_DEBUG.*/# CONFIG_DEBUG is not set/" .config
	sed -i -e "s/^USING_CROSS_COMPILER.*/# USING_CROSS_COMPILER is not set/" .config
	sed -i -e "s/^CONFIG_STATIC.*/# CONFIG_STATIC is not set/" .config
	sed -i -e "s/^CONFIG_SELINUX.*/# CONFIG_SELINUX is not set/" .config
	@./scripts/config/conf -o $(CONFIG_CONFIG_IN)

allnoconfig: scripts/config/conf
	@./scripts/config/conf -n $(CONFIG_CONFIG_IN)

defconfig: scripts/config/conf
	@./scripts/config/conf -d $(CONFIG_CONFIG_IN)

clean:
	- rm -f docs/busybox.dvi docs/busybox.ps \
	    docs/busybox.pod docs/busybox.net/busybox.html \
	    docs/busybox pod2htm* *.gdb *.elf *~ core .*config.log \
	    docs/BusyBox.txt docs/BusyBox.1 docs/BusyBox.html \
	    docs/busybox.net/BusyBox.html busybox.links libbb/loop.h \
	    .config.old .hdepend busybox
	- rm -rf _install
	- find . -name .\*.flags -exec rm -f {} \;
	- find . -name \*.o -exec rm -f {} \;
	- find . -name \*.a -exec rm -f {} \;

distclean: clean
	- rm -f scripts/split-include scripts/mkdep
	- rm -rf include/config include/config.h
	- find . -name .depend -exec rm -f {} \;
	rm -f .config .config.old .config.cmd
	- $(MAKE) -C scripts/config clean

release: distclean #doc
	cd ..; \
	rm -rf $(PROG)-$(VERSION); \
	cp -a busybox $(PROG)-$(VERSION); \
	\
	find $(PROG)-$(VERSION)/ -type d \
		-name .svn \
		-print \
		-exec rm -rf {} \; ; \
	\
	find $(PROG)-$(VERSION)/ -type f \
		-name .\#* \
		-print \
		-exec rm -f {} \; ; \
	\
	tar -cvzf $(PROG)-$(VERSION).tar.gz $(PROG)-$(VERSION)/;

tags:
	ctags -R .


endif # ifeq ($(strip $(HAVE_DOT_CONFIG)),y)

endif # ifeq ($(skip-makefile),)

.PHONY: dummy subdirs release distclean clean config oldconfig \
	menuconfig tags check test depend buildtree
