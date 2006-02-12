#  Copyright 2001, Cybertan Corporation
#  All Rights Reserved.
#  
#  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cybertan Corporation;
#  the contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of Cybertan Corporation.
#
#
# $Id: Makefile,v 1.2 2004/11/12 04:26:43 honor Exp $
#

O_TARGET	:= ctmisc.o

MISC_OBJS	:= misc.o flash.o

export-objs	:= flash.o

obj-y		:= ctmisc.o
obj-m		:= $(O_TARGET)

SRCBASE		:= $(TOPDIR)/../..
EXTRA_CFLAGS	+= -I$(SRCBASE)/include

vpath %.c $(SRCBASE)/shared

obj-y		:= $(foreach obj,$(obj-y),$(SRCBASE)/wl/ctmisc/$(obj))
variant-objs	:= $(foreach obj,$(variant-objs),$(SRCBASE)/wl/ctmisc/$(obj))

include $(TOPDIR)/Rules.make
