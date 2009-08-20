#
# Makefile for the Broadcom EMF Layer
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.3 2008/08/13 20:20:16 Exp $
#
EXTRA_CFLAGS+=-I$(TOPDIR)/brcm/include -I$(TOPDIR)/drivers/net/emf -I$(TOPDIR)/brcm/include/emf/emf -I$(TOPDIR)/brcm/include/emf/igs

EMFSRCDIR := ../../../../../router/emf/emf
EMFDIR := .

EMF_OBJS := emfc.o emf_linux.o

emf-objs := $(EMF_OBJS)

obj-$(CONFIG_EMF) := emf.o

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

clean-files += $(EMFCSRCDIR)/*.o $(EMFSRCDIR)/*.o $(EMFSRCDIR)/.*.o.cmd
