#
# Makefile for the Broadcom EMF Layer
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


EMFSRCDIR := ../../../brcm/arm/emf/linux
EMFCSRCDIR := ../../../brcm/arm/emf/sys
EMFDIR := ../../../brcm/arm/emf/exe

EMF_OBJS := $(EMFCSRCDIR)/emfc.o $(EMFSRCDIR)/emf_linux.o

ifeq ($(CONFIG_BUZZZ_FUNC),y)
subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
endif # CONFIG_BUZZZ_FUNC

SRCBASE := $(srctree)/brcm/arm

# Search for sources under src/router/emf/emf
ifneq ($(wildcard $(src)/$(EMFSRCDIR)),)
	EXTRA_CFLAGS += -I$(SRCBASE)/common/include -I$(SRCBASE)/emf/include 
	EXTRA_CFLAGS += -I$(SRCBASE)/include/emf/emf -I$(SRCBASE)/include/emf/igs
	emf-objs := $(EMF_OBJS)
else
	emf-objs := $(EMFDIR)/emf.o
endif

obj-$(CONFIG_EMF) := emf.o

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

clean-files += $(EMFCSRCDIR)/*.o $(EMFSRCDIR)/*.o $(EMFCSRCDIR)/.*.o.cmd $(EMFSRCDIR)/.*.o.cmd
