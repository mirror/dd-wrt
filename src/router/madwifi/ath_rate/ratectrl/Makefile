#
# Copyright (c) 2004 Sam Leffler, Errno Consulting
# Copyright (c) 2004 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/ratectrl/Makefile#3 $
#

#
# Makefile for the Atheros Rate Control Support.
#
ifeq ($(obj),)
obj=	.
endif

DEPTH=	../../

# release tag versioning
-include $(KERNELPATH)/ath_version.mk
export EXTRAVERSION

obj-m		+= ath_rate_atheros.o
ath_rate_atheros-objs	:= linux.o ratectrl.o ar5212Phy.o ar5211Phy.o
ifeq ($(findstring 2.6,$(KERNELRELEASE)),)
export-objs	:= linux.o
list-multi	:= ath_rate_atheros.o
endif

include $(obj)/${DEPTH}/Makefile.inc

ifneq ($(MAKING_MODULES),1)
obj-$(CONFIG_RATE_ATHEROS) := $(ath_rate_atheros-objs)
ifeq ($(findstring 2.6,$(KERNELRELEASE)),)
O_TARGET := ath_rate_atheros.o
endif
endif

INCS+=	-I${HAL} -I${HAL}/${OS} -I${obj}/${ATH} -I${obj}/${WLAN} \
	-I${obj}/${DEPTH}

EXTRA_CFLAGS+= ${INCS} ${COPTS2}

-include $(TOPDIR)/Rules.make

all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

install: all
	test -d ${DESTDIR}/${MODULEPATH} || mkdir -p ${DESTDIR}/${MODULEPATH}
	${TOOLPREFIX}strip -S ath_rate_atheros.$(KMODSUF)
	cp ath_rate_atheros.${KMODSUF} ${DESTDIR}/${MODULEPATH}

clean:
	-rm -f *~ *.o *.ko *.mod.c
	-rm -f .depend .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd

ath_rate_atheros.o: ${ath_rate_atheros-objs}
	$(LD) $(LDOPTS) -o ath_rate_atheros.$(KMODSUF) -r $(ath_rate_atheros-objs)
