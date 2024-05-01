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
include $(TOP)/.config

ifeq "$(SRCBASE)" ""
	SRCBASE=$(shell cd ../.. ; pwd)
endif

CFG_BCMIGD ?= 1

UPNP	:= $(TOP)/upnp/src/linux
CFLAGS += $(COPTS) -s -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
ifeq ($(strip $(CFG_BCMIGD)),1)
CFLAGS  += -D__BCMIGD__
endif
DEBUG	:= 0

ifeq ($(CONFIG_ANTAIRA),y)
CFLAGS += -DHAVE_ANTAIRA
endif

ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS += -DHAVE_WZRG300NH
else ifeq ($(CONFIG_WZRG300NH2),y)
CFLAGS += -DHAVE_WZRG300NH2
else ifeq ($(CONFIG_WZRG450),y)
CFLAGS += -DHAVE_WZRG450
else ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS += -DHAVE_WZRHPAG300NH
else ifeq ($(CONFIG_WHRHPG300N),y)
CFLAGS += -DHAVE_WHRHPG300N
else ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
endif

CFLAGS += $(MIPS16_OPT) $(THUMB) $(LTO)
LDFLAGS += $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections

all: upnp

install: all
	install -d $(INSTALLDIR)/upnp/usr/sbin
	install -m 755 bcmupnp $(INSTALLDIR)/upnp/usr/sbin/upnp

clean:
	make -C $(UPNP) clean
	rm -f bcmupnp

upnp: FORCE
	make -C $(UPNP) DEBUG=$(DEBUG) SRCBASE=$(SRCBASE) TARGET_PREFIX=$(CROSS_COMPILE) CFG_BCMIGD=$(CFG_BCMIGD) ADDLIBS="$(ADDLIBS)"
	cp $(UPNP)/upnp ./bcmupnp

FORCE:
