#
# Makefile for the Broadcom et driver
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.1 2005/03/16 13:50:00 wbx Exp $
#

EXTRA_CFLAGS := -I$(TOPDIR)/arch/mips/bcm947xx/include -DBCM47XX_CHOPS -DDMA -DBCMDRIVER

O_TARGET	:= robo.o
ROBO_OBJS	:= brcmtag.o if_robo.o igmp.o osl_linux.o swapi.o swmod.o
obj-m		:= $(O_TARGET)

obj-y		:= $(foreach obj,$(ROBO_OBJS),$(SRCBASE)/robo/linux/$(obj)) 
obj-y		+= semaphore.o
include $(TOPDIR)/Rules.make
