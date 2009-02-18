#
# Makefile for the Broadcom wl driver
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.2 2005/03/29 03:32:18 mbm Exp $


EXTRA_CFLAGS += -I$(TOPDIR)/arch/mips/bcm947xx/include -DBCMDRIVER=1 -DBCMDMA64=1

O_TARGET	:= wl.o

ifdef CONFIG_V25
obj-$(CONFIG_APSTA)		:= wl_apsta.v25.o_save
obj-$(CONFIG_APSTAMIMO)		:= wl_apsta_mimo.v25.o_save
else
obj-$(CONFIG_APSTA)		:= wl_apsta.o_save
obj-$(CONFIG_APSTAMIMO)		:= wl_apsta_mimo.o_save
endif

obj-y		+= hnddma.o 
ifeq ($(CONFIG_WL),m)
obj-m		:= $(O_TARGET)
endif


include $(TOPDIR)/Rules.make
