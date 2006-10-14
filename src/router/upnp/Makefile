#
# Linux upnp Makefile
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.1.1.6 2004/04/12 04:34:09 honor Exp $
#
include $(TOP)/.config

UPNP	:= $(SRCBASE)/tools/upnp/igd/linux
#CFLAGS	+= -Os -mips32 -mtune=mips32 -pipe -funit-at-a-time
CFLAGS	+= -s $(COPTS)
#DEBUG	:= 1

all: upnp

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -m 755 upnp $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/upnp

clean:
	rm -f upnp
	$(MAKE) -C $(UPNP) clean

upnp: FORCE
	$(MAKE) -C $(UPNP) DEBUG=$(DEBUG) SRCBASE=$(SRCBASE) TARGET_PREFIX=$(CROSS_COMPILE)
	cp $(UPNP)/upnp .
#	cp $(UPNP)/../../upnp/linux/libupnp.a .

FORCE:
