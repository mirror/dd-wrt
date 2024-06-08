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

CFLAGS	+= -I. -I$(TOP)/shared

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)
CFLAGS +=$(call cc-option,-Werror=return-local-addr,)



#CFLAGS	+= -g -DDEBUG
CFLAGS	+= $(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC
LDFLAGS += -L.
ifeq ($(ARCH),powerpc)
#CFLAGS  += -DHAVE_MAGICBOX -fpic
endif
ifeq ($(ARCHITECTURE),gateworx)
CFLAGS  += -DHAVE_GATEWORX
CFLAGS  += -DHAVE_XSCALE
endif
ifeq ($(ARCHITECTURE),laguna)
CFLAGS  += -DHAVE_GATEWORX
CFLAGS  += -DHAVE_LAGUNA
endif

ifeq ($(CONFIG_MVEBU),y)
CFLAGS += -DHAVE_MVEBU
endif
ifeq ($(CONFIG_IPQ806X),y)
CFLAGS += -DHAVE_IPQ806X
endif
ifeq ($(CONFIG_IPQ6018),y)
CFLAGS += -DHAVE_IPQ6018
endif
ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif
ifeq ($(CONFIG_EROUTER),y)
CFLAGS += -DHAVE_EROUTER
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
ifeq ($(CONFIG_UBNTM),y)
CFLAGS += -DHAVE_UBNTM
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
ifeq ($(CONFIG_NEWPORT),y)
CFLAGS += -DHAVE_NEWPORT
endif
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -DHAVE_ALPINE
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -DHAVE_RB600
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
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
LDFLAGS += -lpthread
endif
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

%.o : %.c
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

all: libnvram.so async_commit

install: all
	install -D libnvram.so $(INSTALLDIR)/lib/libnvram.so
	install -D async_commit $(INSTALLDIR)/sbin/async_commit
	$(STRIP) $(INSTALLDIR)/lib/libnvram.so


clean:
	rm -f *.o *.a *.so nvram async_commit

async_commit: async_commit.o
	@$(CC) $(CFLAGS) -o $@ $^ -L. -lnvram
	

libnvram.so: nvram_linux.o nvram_convert.o backup.o
	@echo "[LD] $@"
	echo $(LDFLAGS) > libnvram_ld
	@$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)
	@$(AR) cr libnvram.a $^

convertnvram: convertnvram.o
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
