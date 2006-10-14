#
# Makefile	Main Makefile for the net-tools Package
#
# NET-TOOLS	A collection of programs that form the base set of the
#		NET-3 Networking Distribution for the LINUX operating
#		system.
#
# Version:	2001-02-13
#
# Author:	Bernd Eckenfels <net-tools@lina.inka.de>
#		Copyright 1995-1996 Bernd Eckenfels, Germany
#
# URLs:		ftp://ftp.inka.de/pub/comp/Linux/networking/NetTools/ 
#		ftp://ftp.linux.org.uk/pub/linux/Networking/PROGRAMS/NetTools/
#		http://www.inka.de/sites/lina/linux/NetTools/index_en.html
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
#	{1.20}	Bernd Eckenfels:	Even more modifications for the new 
#					package layout
#	{1.21}	Bernd Eckenfels:	Check if config.in is newer than 
#					config.status
#	{1.22}  Bernd Eckenfels:	Include ypdomainname and nisdomainame
#
#	1.3.50-BETA6 private Release
#				
#960125	{1.23}	Bernd Eckenfels:	Peter Tobias' rewrite for 
#					makefile-based installation
#	1.3.50-BETA6a private Release
#
#960201 {1.24}	Bernd Eckenfels:	net-features.h added
#
#960201 1.3.50-BETA6b private Release
#
#960203 1.3.50-BETA6c private Release
#
#960204 1.3.50-BETA6d private Release
#
#960204 {1.25}	Bernd Eckenfels:	DISTRIBUTION added
#
#960205 1.3.50-BETA6e private Release
#
#960206	{1.26}	Bernd Eckenfels:	afrt.o removed (cleaner solution)
#
#960215 1.3.50-BETA6f Release
#
#960216 {1.30}	Bernd Eckenfels:	net-lib support
#960322 {1.31}	Bernd Eckenfels:	moveable netlib, TOPDIR
#960424 {1.32}	Bernd Eckenfels:	included the URLs in the Comment
#
#960514 1.31-alpha release
#
#960518 {1.33}	Bernd Eckenfels:	-I/usr/src/linux/include comment added
#
#	This program is free software; you can redistribute it
#	and/or  modify it under  the terms of  the GNU General
#	Public  License as  published  by  the  Free  Software
#	Foundation;  either  version 2 of the License, or  (at
#	your option) any later version.
#

# set the base of the Installation 
# BASEDIR = /mnt

# path to the net-lib support library. Default: lib
NET_LIB_PATH = lib
NET_LIB_NAME = net-tools

PROGS	:= ifconfig

-include config.make
ifeq ($(HAVE_IP_TOOLS),1)
PROGS   += iptunnel ipmaddr
endif
ifeq ($(HAVE_MII),1)
PROGS	+= mii-tool
endif

#CC = mipsel-linux-uclibc-gcc
# Compiler and Linker Options
# You may need to uncomment and edit these if you are using libc5 and IPv6.
COPTS += -D_GNU_SOURCE -Wall -DNEED_PRINTF # -I/usr/inet6/include
ifeq ($(origin LOPTS), undefined)
LOPTS = 
endif
RESLIB = # -L/usr/inet6/lib -linet6

ifeq ($(HAVE_AFDECnet),1)
DNLIB = -ldnet
endif

# -------- end of user definitions --------

MAINTAINER = Philip.Blundell@pobox.com
RELEASE	   = 1.60

.EXPORT_ALL_VARIABLES:

ifeq ("$(NET_LIB_PATH)","lib2")
TOPDIR   = ..
else
TOPDIR  := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
endif

NET_LIB = $(NET_LIB_PATH)/lib$(NET_LIB_NAME).a

CFLAGS	= $(COPTS) -I. -idirafter ./include/ -I$(NET_LIB_PATH)
LDFLAGS	= $(LOPTS) -L$(NET_LIB_PATH)

SUBDIRS	= man/ $(NET_LIB_PATH)/

ifeq ($(origin CC), undefined)
CC	= gcc
endif
LD	= $(CC)

NLIB	= -l$(NET_LIB_NAME)

MDEFINES = COPTS='$(COPTS)' LOPTS='$(LOPTS)' TOPDIR='$(TOPDIR)'

%.o:		%.c config.h version.h intl.h net-features.h $<
		$(CC) $(CFLAGS) -c $<

all:		config.h version.h subdirs $(PROGS)

config: 	cleanconfig config.h

install:	all savebin installbin installdata

update: 	all installbin installdata

mostlyclean:
		rm -f *.o DEADJOE config.new *~ *.orig lib/*.o

clean: mostlyclean
		rm -f *.a
		rm -f $(PROGS)
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) ; done
		@cd po && $(MAKE) clean

cleanconfig:
		rm -f config.h

clobber: 	clean
		rm -f $(PROGS) config.h version.h config.status config.make
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clobber) ; done


dist: 		clobber
		@echo Creating net-tools-$(RELEASE) in ..
		@tar -cvz -f ../net-tools-$(RELEASE).tar.gz -C .. net-tools


config.h: 	config.in Makefile 
		@echo "Configuring the Linux net-tools (NET-3 Base Utilities)..." ; echo
		@if [ config.status -nt config.in ]; \
			then ./configure.sh config.status; \
		   else ./configure.sh config.in; \
		 fi


version.h:	Makefile
		@echo "#define RELEASE \"net-tools $(RELEASE)\"" >version.h


$(NET_LIB):	config.h version.h intl.h libdir

i18n.h:		i18ndir

libdir:
		@$(MAKE) -C $(NET_LIB_PATH) $(MDEFINES)

i18ndir:
		@$(MAKE) -C po

subdirs:
		@for i in $(SUBDIRS); do $(MAKE) -C $$i $(MDEFINES) ; done

ifconfig:	$(NET_LIB) ifconfig.o
		$(CC) $(LDFLAGS) -o ifconfig ifconfig.o $(NLIB) $(RESLIB)
		
nameif:	nameif.o
		$(CC) $(LDFLAGS) -o nameif nameif.o 

hostname:	hostname.o
		$(CC) $(LDFLAGS) -o hostname hostname.o $(DNLIB)

route:		$(NET_LIB) route.o
		$(CC) $(LDFLAGS) -o route route.o $(NLIB) $(RESLIB)

arp:		$(NET_LIB) arp.o
		$(AR) arc -o arp.a arp.o

rarp:		$(NET_LIB) rarp.o
		$(CC) $(LDFLAGS) -o rarp rarp.o $(NLIB)

slattach:	$(NET_LIB) slattach.o
		$(CC) $(LDFLAGS) -o slattach slattach.o $(NLIB)

plipconfig:	$(NET_LIB) plipconfig.o
		$(CC) $(LDFLAGS) -o plipconfig plipconfig.o $(NLIB)

netstat:	$(NET_LIB) netstat.o statistics.o
		$(CC) $(LDFLAGS) -o netstat netstat.o statistics.o $(NLIB) $(RESLIB)

iptunnel:	$(NET_LIB) iptunnel.o
		$(CC) $(LDFLAGS) -o iptunnel iptunnel.o $(NLIB) $(RESLIB)

ipmaddr:	$(NET_LIB) ipmaddr.o
		$(CC) $(LDFLAGS) -o ipmaddr ipmaddr.o $(NLIB) $(RESLIB)

mii-tool:	mii-tool.o
		$(CC) $(LDFLAGS) -o mii-tool mii-tool.o

installbin:
	install -m 0755 -d ${BASEDIR}/sbin
	install -m 0755 -d ${BASEDIR}/bin
	install -m 0755 arp        ${BASEDIR}/sbin
	install -m 0755 hostname   ${BASEDIR}/bin
	install -m 0755 ifconfig   ${BASEDIR}/sbin
	install -m 0755 nameif     ${BASEDIR}/sbin
	install -m 0755 netstat    ${BASEDIR}/bin
	install -m 0755 plipconfig $(BASEDIR)/sbin
	install -m 0755 rarp       ${BASEDIR}/sbin
	install -m 0755 route      ${BASEDIR}/sbin
	install -m 0755 slattach   $(BASEDIR)/sbin
ifeq ($(HAVE_IP_TOOLS),1)
	install -m 0755 ipmaddr    $(BASEDIR)/sbin
	install -m 0755 iptunnel   $(BASEDIR)/sbin
endif
ifeq ($(HAVE_MII),1)
	install -m 0755 mii-tool   $(BASEDIR)/sbin
endif
	ln -fs hostname $(BASEDIR)/bin/dnsdomainname
	ln -fs hostname $(BASEDIR)/bin/ypdomainname
	ln -fs hostname $(BASEDIR)/bin/nisdomainname
	ln -fs hostname $(BASEDIR)/bin/domainname
ifeq ($(HAVE_AFDECnet),1)
	ln -fs hostname $(BASEDIR)/bin/nodename
endif

savebin:
	@for i in ${BASEDIR}/sbin/arp ${BASEDIR}/sbin/ifconfig \
                 ${BASEDIR}/bin/netstat \
		 ${BASEDIR}/sbin/rarp ${BASEDIR}/sbin/route \
		 ${BASEDIR}/bin/hostname ${BASEDIR}/bin/ypdomainname \
                 ${BASEDIR}/bin/dnsdomainname ${BASEDIR}/bin/nisdomainname \
		 ${BASEDIR}/bin/domainname ; do \
		 [ -f $$i ] && cp -f $$i $$i.old ; done ; echo Saved.

installdata:
	$(MAKE) -C man install
	$(MAKE) -C po install

# End of Makefile.
