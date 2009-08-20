#
# Makefile for the Broadcom et driver
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.3 2008/08/27 04:46:49 Exp $
#

EXTRA_CFLAGS+=-I$(TOPDIR)/brcm/include 
ETSRCDIR := .

et-objs := $(ETSRCDIR)/sys/et_linux.o $(ETSRCDIR)/sys/etc.o

ifeq ($(CONFIG_ET_47XX),y)
et-objs		+= $(ETSRCDIR)/sys/etc47xx.o $(ETSRCDIR)/sys/etc_adm.o
et-objs		+= $(ETSRCDIR)/sys/etcgmac.o
EXTRA_CFLAGS	+= -DCFG_GMAC -DBCM47XX_CHOPS -DBCMDMA64 -DETROBO -DETADM 
endif

obj-$(CONFIG_ET) := et.o

EXTRA_CFLAGS	+= -DDMA 

# Search for sources under src/et/sys or objects under src/et/linux
ifneq ($(wildcard $(src)/$(ETSRCDIR)/sys),)
EXTRA_CFLAGS	+= -I$(src)/$(ETSRCDIR)/sys
else
et-objs := $(ETSRCDIR)/linux/et.o
endif

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
else
# Normal makefile
KERNELDIR := ../../../../../linux/linux-2.6
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd`

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
endif

clean-files += $(ETSRCDIR)/sys/*.o $(ETSRCDIR)/sys/.*.o.cmd
