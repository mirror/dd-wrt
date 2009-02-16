#
# Linux upnp Makefile support WFA device
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.4 2008/12/02 08:12:11 Exp $
#

ifeq "$(SRCBASE)" ""
	SRCBASE=$(shell cd ../.. ; pwd)
endif

CFG_BCMIGD ?= 1

UPNP	:= $(TOP)/upnp/src/linux
CFLAGS += -Os
ifeq ($(strip $(CFG_BCMIGD)),1)
CFLAGS  += -D__BCMIGD__
endif
#DEBUG	:= 1

all: upnp

install: all
	install -d $(INSTALLDIR)/upnp/usr/sbin
	install -m 755 bcmupnp $(INSTALLDIR)/upnp/usr/sbin/upnp

clean:
	$(MAKE) -C $(UPNP) clean
	rm -f bcmupnp

upnp: FORCE
	$(MAKE) -C $(UPNP) DEBUG=$(DEBUG) SRCBASE=$(SRCBASE) TARGET_PREFIX=$(CROSS_COMPILE) CFG_BCMIGD=$(CFG_BCMIGD)
	cp $(UPNP)/upnp ./bcmupnp

FORCE:
