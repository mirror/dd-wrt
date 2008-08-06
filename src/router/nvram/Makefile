#
# Linux NVRAM Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.5 2003/10/29 03:43:55 honor Exp $
#


include $(TOP)/.config

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	+= -I. -I$(SRCBASE)/include.bcm -Wall
else
CFLAGS	+= -I. -I$(SRCBASE)/include.v24 -Wall
endif
else
CFLAGS	+= -I. -I$(SRCBASE)/include.v23 -Wall
endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s $(COPTS) -fpic
LDFLAGS += -L.
ifeq ($(ARCH),powerpc)
CFLAGS  += -DHAVE_MAGICBOX -fpic
endif
ifeq ($(ARCHITECTURE),gateworx)
CFLAGS  += -DHAVE_GATEWORX
CFLAGS  += -DHAVE_XSCALE
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif
ifeq ($(CONFIG_FONERA),y)
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_WHRAG108),y)
CFLAGS += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS += -DHAVE_TW6600
endif
ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif

vpath %.c $(SRCBASE)/shared/nvram

all: libnvram.so convertnvram

install: all
	install -D libnvram.so $(INSTALLDIR)/lib/libnvram.so
	$(STRIP) $(INSTALLDIR)/lib/libnvram.so
ifeq ($(ARCH),armeb)
	install -D convertnvram $(INSTALLDIR)/usr/sbin/convertnvram
endif
ifeq ($(ARCH),powerpc)
	install -D convertnvram $(INSTALLDIR)/usr/sbin/convertnvram
endif
ifeq ($(ARCH),mips)
	install -D convertnvram $(INSTALLDIR)/usr/sbin/convertnvram
endif


clean:
	rm -f *.o *.a *.so nvram

ifeq ($(ARCH),i386)
libnvram.so: nvram_linux_gen.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
ifeq ($(ARCH),armeb)
libnvram.so: nvram_linux.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
ifeq ($(ARCH),arm)
libnvram.so: nvram_linux.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
ifeq ($(ARCH),powerpc)
libnvram.so: nvram_linux.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
ifeq ($(ARCH),mips)
libnvram.so: nvram_linux.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
ifeq ($(ARCH),mipsel)
libnvram.so: nvram_linux.o nvram_convert.o
	$(LD) -shared -o $@ $^
endif
convertnvram: convertnvram.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
nvram: main.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
