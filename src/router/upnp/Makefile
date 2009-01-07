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
CFLAGS	= -s -O2
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_GATEWORX
endif

all: upnp

install: all
	install -d $(INSTALLDIR)/upnp/usr/sbin
	install -m 755 upnp $(INSTALLDIR)/upnp/usr/sbin
	$(STRIP) $(INSTALLDIR)/upnp/usr/sbin/upnp

clean:
	rm -f upnp
	$(MAKE) -C $(UPNP) clean

upnp: FORCE
	$(MAKE) -C $(UPNP) DEBUG=$(DEBUG) SRCBASE=$(SRCBASE) TARGET_PREFIX=$(CROSS_COMPILE)
	cp $(UPNP)/upnp .

FORCE:
