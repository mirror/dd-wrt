#
# Makefile for the Broadcom IGS Layer
#
# Copyright (C) 2013, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: Makefile,v 1.3 2008-08-13 20:20:16 $
#

EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux

IGSSRCDIR := ../../../brcm/arm/igs/linux
IGSCSRCDIR := ../../../brcm/arm/igs/sys
IGSDIR := ../../../brcm/arm/igs/exe

IGS_OBJS := $(IGSCSRCDIR)/igsc.o $(IGSCSRCDIR)/igsc_sdb.o $(IGSSRCDIR)/igs_linux.o \
	    $(IGSCSRCDIR)/osl_linux.o

SRCBASE := $(srctree)/brcm/arm

# Search for sources under src/router/emf/igs
ifneq ($(wildcard $(src)/$(IGSSRCDIR)),)
	EXTRA_CFLAGS += -I$(SRCBASE)/common/include -I$(SRCBASE)/igs/include
	EXTRA_CFLAGS += -I$(SRCBASE)/include/emf/igs -I$(SRCBASE)/include/emf/emf
	igs-objs := $(IGS_OBJS)
else
	igs-objs := $(EMFDIR)/igs.o
endif

obj-$(CONFIG_EMF) := igs.o

ifeq ($(CONFIG_BUZZZ_FUNC),y)
subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
endif # CONFIG_BUZZZ_FUNC

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
else
# Normal makefile
KERNELDIR := $(BASEDIR)/components/opensource/linux/linux-2.6.36
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd`
clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
endif

clean-files += $(IGSCSRCDIR)/*.o $(IGSSRCDIR)/*.o $(IGSCSRCDIR)/.*.o.cmd $(IGSSRCDIR)/.*.o.cmd
