#  Copyright 2001, Broadcom Corporation
#  All Rights Reserved.
#  
#  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
#  the contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of Broadcom Corporation.
#
# Makefile for the Broadcom et driver
#
# $Id: Makefile,v 1.1.1.1 2004/01/12 06:30:41 cvsadm Exp $
#

O_TARGET	:= led.o

LED_OBJS	:= LEDs.o

export-objs	:=
obj-y		:= $(LED_OBJS)
obj-m		:= $(O_TARGET)

SRCBASE		:= $(TOPDIR)/../../../
EXTRA_CFLAGS	+= -I$(SRCBASE)/include.v23
EXTRA_CFLAGS	+= -DDMA

# Search for sources under src/et/sys or objects under src/et/linux
ifneq ($(wildcard $(SRCBASE)/led/sys),)
EXTRA_CFLAGS	+= -I$(SRCBASE)/led/sys
vpath %.c $(SRCBASE)/led/sys $(SRCBASE)/shared
else
obj-y		:= $(foreach obj,$(LED_OBJS),$(SRCBASE)/led/linux/$(obj))
endif

include $(TOPDIR)/Rules.make
