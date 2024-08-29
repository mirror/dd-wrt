#
# Makefile	Main Makefile for the net-tools Package
#
# NET-TOOLS	A collection of programs that form the base set of the
#		NET-3 Networking Distribution for the LINUX operating
#		system.
#
# Author:	Bernd Eckenfels <net-tools@lina.inka.de>
#		Copyright 1995-1996 Bernd Eckenfels, Germany
#
# URLs:		http://net-tools.sourceforge.net/
#
# Based on:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
#		Copyright 1988-1993 MicroWalt Corporation
#
# Modifications:
#		Extensively modified from 01/21/94 onwards by
#		Alan Cox <A.Cox@swansea.ac.uk>
#		Copyright 1993-1994 Swansea University Computer Society
#
# Be careful!
# This Makefile doesn't describe complete dependencies for all include files.
# If you change include files you might need to do make clean.
#

# set the base of the Installation
# BASEDIR = /mnt
BASEDIR ?= $(DESTDIR)
BINDIR ?= /bin
SBINDIR ?= /sbin

# path to the net-lib support library. Default: lib
NET_LIB_PATH = lib
NET_LIB_NAME = net-tools

PROGS	:= ifconfig netstat route nameif

-include config.make
ifeq ($(HAVE_ARP_TOOLS),1)
PROGS	+= arp rarp
endif
ifeq ($(HAVE_HOSTNAME_TOOLS),1)
PROGS	+= hostname
endif
ifeq ($(HAVE_IP_TOOLS),1)
PROGS   += iptunnel ipmaddr
endif
ifeq ($(HAVE_MII),1)
PROGS	+= mii-tool
endif
ifeq ($(HAVE_PLIP_TOOLS),1)
PROGS	+= plipconfig
endif
ifeq ($(HAVE_SERIAL_TOOLS),1)
PROGS	+= slattach
endif

# Compiler and Linker Options
#CFLAGS ?= -O2 -g
CFLAGS += -Wall -DNEED_PRINTF $(LTO) $(MIPS16_OPT) $(THUMB)
CFLAGS += -fno-strict-aliasing # code needs a lot of work before strict aliasing is safe
CPPFLAGS += -D_GNU_SOURCE
# Turn on transparent support for LFS
CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE  $(LTO) $(MIPS16_OPT) $(THUMB)

ifeq ($(HAVE_AFDECnet),1)
DNLIB = -ldnet
endif

ifeq ($(origin CC), undefined)
CC	= gcc
endif
LD	= $(CC)
PKG_CONFIG ?= pkg-config

# -------- end of user definitions --------

MAINTAINER = net-tools-devel@lists.sourceforge.net
RELEASE	   = 2.10

.EXPORT_ALL_VARIABLES:

ifeq ("$(NET_LIB_PATH)","lib2")
TOPDIR   = ..
else
TOPDIR  := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
endif

NET_LIB = $(NET_LIB_PATH)/lib$(NET_LIB_NAME).a

ifeq ($(HAVE_SELINUX),1)
SE_PC_CFLAGS := $(shell $(PKG_CONFIG) --cflags libselinux)
SE_PC_LIBS := $(shell $(PKG_CONFIG) --libs libselinux || echo -lselinux)
SELIB = $(SE_PC_LIBS)
CPPFLAGS += $(SE_PC_CFLAGS)
endif

CPPFLAGS += -I. -I$(TOPDIR)/include -I$(NET_LIB_PATH)
LDFLAGS  += -L$(NET_LIB_PATH) $(LDLTO)

SUBDIRS	= man/ $(NET_LIB_PATH)/

NLIB	= -l$(NET_LIB_NAME)

%.o:		%.c config.h version.h intl.h lib/net-features.h $<
		$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

all:		config.h version.h subdirs $(PROGS)

config: 	cleanconfig config.h

install:	all installbin installdata

mostlyclean:
		rm -f -- *.o DEADJOE config.new *~ *.orig lib/*.o

clean: mostlyclean
		rm -f -- $(PROGS)
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) ; done
		@cd po && $(MAKE) clean

cleanconfig:
		rm -f -- config.h

clobber: 	clean
		rm -f -- $(PROGS) config.h version.h config.status config.make
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clobber) ; done


dist:
		rm -rf -- net-tools-$(RELEASE)
		git archive --prefix=net-tools-$(RELEASE)/ HEAD | tar xf -
		$(MAKE) -C net-tools-$(RELEASE)/po $@
		tar cf - net-tools-$(RELEASE)/ | xz > net-tools-$(RELEASE).tar.xz
		rm -rf -- net-tools-$(RELEASE)

distcheck:	dist
		tar xf net-tools-$(RELEASE).tar.xz
		yes "" | $(MAKE) -C net-tools-$(RELEASE) config
		$(MAKE) -C net-tools-$(RELEASE)
		rm -rf -- net-tools-$(RELEASE)
		@printf "\nThe tarball is ready to go:\n%s\n" "`du -b net-tools-$(RELEASE).tar.xz`"

config.h: 	config.in Makefile
		@echo "Configuring the Linux net-tools (NET-3 Base Utilities)..." ; echo
#		@if [ config.status -nt config.in ]; \
#			then ./configure.sh config.status; \
#		   else ./configure.sh config.in; \
#		 fi


version.h:	Makefile
		@echo "#define RELEASE \"net-tools $(RELEASE)\"" >version.h


$(NET_LIB):	config.h version.h intl.h libdir

i18n.h:		i18ndir

libdir:		version.h
		@$(MAKE) -C $(NET_LIB_PATH)

i18ndir:
		@$(MAKE) -C po

# use libdir target for lib/ to avoid parallel build issues
subdirs:	libdir
		@for i in $(SUBDIRS:$(NET_LIB_PATH)/=); do $(MAKE) -C $$i || exit $$? ; done

ifconfig:	$(NET_LIB) ifconfig.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ifconfig.o $(NLIB) $(LDLIBS)

nameif:		$(NET_LIB) nameif.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ nameif.o $(NLIB) $(LDLIBS)

hostname:	hostname.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ hostname.o $(DNLIB) $(LDLIBS)

route:		$(NET_LIB) route.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ route.o $(NLIB) $(LDLIBS)

arp:		$(NET_LIB) arp.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ arp.o $(NLIB) $(LDLIBS)

rarp:		$(NET_LIB) rarp.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ rarp.o $(NLIB) $(LDLIBS)

slattach:	$(NET_LIB) slattach.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ slattach.o $(NLIB) $(LDLIBS)

plipconfig:	$(NET_LIB) plipconfig.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ plipconfig.o $(NLIB) $(LDLIBS)

netstat:	$(NET_LIB) netstat.o statistics.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ netstat.o statistics.o $(NLIB) $(SELIB) $(LDLIBS)

iptunnel:	$(NET_LIB) iptunnel.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ iptunnel.o $(NLIB) $(LDLIBS)

ipmaddr:	$(NET_LIB) ipmaddr.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ipmaddr.o $(NLIB) $(LDLIBS)

mii-tool:	$(NET_LIB) mii-tool.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ mii-tool.o $(NLIB) $(LDLIBS)

installbin:
	@echo
	@echo "######################################################"
	@echo "Notice: ifconfig and route are now installed into /bin"
	@echo "######################################################"
	@echo
	install -m 0755 -d ${BASEDIR}${SBINDIR}
	install -m 0755 -d ${BASEDIR}${BINDIR}
	install -m 0755 ifconfig   ${BASEDIR}${BINDIR}
	install -m 0755 nameif     ${BASEDIR}${SBINDIR}
	install -m 0755 netstat    ${BASEDIR}${BINDIR}
	install -m 0755 route      ${BASEDIR}${BINDIR}
ifeq ($(HAVE_ARP_TOOLS),1)
	install -m 0755 arp        ${BASEDIR}${SBINDIR}
	install -m 0755 rarp       ${BASEDIR}${SBINDIR}
endif
ifeq ($(HAVE_HOSTNAME_TOOLS),1)
	install -m 0755 hostname   ${BASEDIR}${BINDIR}
	ln -fs hostname $(BASEDIR)${BINDIR}/dnsdomainname
ifeq ($(HAVE_HOSTNAME_SYMLINKS),1)
	ln -fs hostname $(BASEDIR)${BINDIR}/ypdomainname
	ln -fs hostname $(BASEDIR)${BINDIR}/nisdomainname
	ln -fs hostname $(BASEDIR)${BINDIR}/domainname
endif
ifeq ($(HAVE_AFDECnet),1)
	ln -fs hostname $(BASEDIR)${BINDIR}/nodename
endif
endif
ifeq ($(HAVE_IP_TOOLS),1)
	install -m 0755 ipmaddr    $(BASEDIR)${SBINDIR}
	install -m 0755 iptunnel   $(BASEDIR)${SBINDIR}
endif
ifeq ($(HAVE_MII),1)
	install -m 0755 mii-tool   $(BASEDIR)${SBINDIR}
endif
ifeq ($(HAVE_PLIP_TOOLS),1)
	install -m 0755 plipconfig $(BASEDIR)${SBINDIR}
endif
ifeq ($(HAVE_SERIAL_TOOLS),1)
	install -m 0755 slattach   $(BASEDIR)${SBINDIR}
endif

installdata:
	$(MAKE) -C man install
	$(MAKE) -C po install

# End of Makefile.
