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
CFLAGS += -Os -s -I$(LINUXDIR)/brcm/shared/bcmwifi/include
ifeq ($(strip $(CFG_BCMIGD)),1)
CFLAGS  += -D__BCMIGD__
endif
DEBUG	:= 0

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

CFLAGS += $(MIPS16_OPT)

ifeq ($(CONFIG_TIEXTRA1),y)
ADDLIBS =  -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
ADDLIBS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_TIEXTRA2),y)
ADDLIBS =  -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
ADDLIBS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_SAMBA3),y)
ADDLIBS =  -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
ADDLIBS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_MINIDLNA),y)
ADDLIBS =  -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
ADDLIBS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_FTP),y)
ADDLIBS =  -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
ADDLIBS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif

all: upnp

install: all
	install -d $(INSTALLDIR)/upnp/usr/sbin
	install -m 755 bcmupnp $(INSTALLDIR)/upnp/usr/sbin/upnp

clean:
	$(MAKE) -C $(UPNP) clean
	rm -f bcmupnp

upnp: FORCE
	$(MAKE) -C $(UPNP) DEBUG=$(DEBUG) SRCBASE=$(SRCBASE) TARGET_PREFIX=$(CROSS_COMPILE) CFG_BCMIGD=$(CFG_BCMIGD) ADDLIBS="$(ADDLIBS)"
	cp $(UPNP)/upnp ./bcmupnp

FORCE:
