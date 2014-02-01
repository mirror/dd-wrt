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

ifeq ($(CONFIG_80211AC),y)
CFLAGS	+= -I. -I$(LINUXDIR)/brcm/$(ARCH)/include -Wall
else
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	+= -I. -I$(SRCBASE)/include.bcm -Wall
else
CFLAGS	+= -I. -I$(SRCBASE)/include.v24 -Wall
endif
else
CFLAGS	+= -I. -I$(SRCBASE)/include.v23 -Wall
endif
endif


#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s $(COPTS) -fPIC
LDFLAGS += -L.
ifeq ($(ARCH),powerpc)
CFLAGS  += -DHAVE_MAGICBOX -fpic
endif
ifeq ($(ARCHITECTURE),gateworx)
CFLAGS  += -DHAVE_GATEWORX
CFLAGS  += -DHAVE_XSCALE
endif
ifeq ($(ARCHITECTURE),laguna)
CFLAGS  += -DHAVE_GATEWORX
CFLAGS  += -DHAVE_LAGUNA
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif
ifeq ($(CONFIG_FONERA),y)
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_RT2880),y)
CFLAGS += -DHAVE_RT2880
endif
ifeq ($(CONFIG_RT3052),y)
CFLAGS += -DHAVE_RT3052
endif
ifeq ($(CONFIG_VENTANA),y)
CFLAGS += -DHAVE_VENTANA
endif
ifeq ($(CONFIG_STORM),y)
CFLAGS += -DHAVE_STORM
endif
ifeq ($(CONFIG_WHRAG108),y)
CFLAGS += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_NORTHSTAR),y)
CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS += -DHAVE_TW6600
endif
ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
endif
ifeq ($(CONFIG_LSX),y)
CFLAGS += -DHAVE_LSX
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_DIR825),y)
CFLAGS += -DHAVE_DIR825
endif
ifeq ($(CONFIG_WRT400),y)
CFLAGS += -DHAVE_WRT400
endif
ifeq ($(CONFIG_WZRG450),y)
CFLAGS += -DHAVE_WZRG450
endif
ifeq ($(CONFIG_DANUBE),y)
CFLAGS += -DHAVE_DANUBE
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif
ifeq ($(CONFIG_WDR4900),y)
CFLAGS += -DHAVE_WDR4900
endif
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS += -DHAVE_WZRHPAG300NH
endif
ifeq ($(CONFIG_TEW632BRP),y)
CFLAGS += -DHAVE_TEW632BRP
endif
ifeq ($(CONFIG_TG2521),y)
CFLAGS += -DHAVE_TG2521
endif
ifeq ($(CONFIG_WR1043),y)
CFLAGS += -DHAVE_WR1043
endif
ifeq ($(CONFIG_JJAP93),y)
CFLAGS += -DHAVE_JJAP93
endif
ifeq ($(CONFIG_WR741),y)
CFLAGS += -DHAVE_WR741
endif
ifeq ($(CONFIG_DIR615I),y)
CFLAGS += -DHAVE_DIR615I
endif
ifeq ($(CONFIG_NVRAM_60K),y)
CFLAGS += -DHAVE_NVRAM_60K
endif
ifeq ($(CONFIG_NVRAM_64K),y)
CFLAGS += -DHAVE_NVRAM_64K
endif

include $(TOP)/rules/generic.mk
CFLAGS += $(DD_CFLAGS)

vpath %.c $(SRCBASE)/shared/nvram

all: libnvram.so 

install: all
	install -D libnvram.so $(INSTALLDIR)/lib/libnvram.so
	$(STRIP) $(INSTALLDIR)/lib/libnvram.so


clean:
	rm -f *.o *.a *.so nvram

ifeq ($(ARCH),i386)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),x86_64)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),armeb)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),arm)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),powerpc)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),mips)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
ifeq ($(ARCH),mipsel)
libnvram.so: nvram_linux.o nvram_convert.o backup.o
	$(CC) $(CFLAGS) -shared -o $@ $^
	$(AR) cr libnvram.a $^
endif
convertnvram: convertnvram.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
nvram: main.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
