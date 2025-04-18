#
# Makefile for the Broadcom et driver
#
# Copyright (C) 2015, Broadcom Corporation. All Rights Reserved.
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
# $Id: Makefile,v 1.5 2010-12-07 04:47:36 $
#

EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/common/include -DBCMDBG -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/common/include -DBCMDBG -DBCMDRIVER -Dlinux
ETSRCDIR := ../../../../brcm/arm/et

et-objs := $(ETSRCDIR)/sys/et_linux.o $(ETSRCDIR)/sys/etc.o $(ETSRCDIR)/sys/et_glue.o

ifeq ($(CONFIG_ET_47XX),y)
et-objs += $(ETSRCDIR)/sys/etc47xx.o $(ETSRCDIR)/sys/etc_adm.o
et-objs += $(ETSRCDIR)/sys/etcgmac.o
EXTRA_CFLAGS	+= -DCFG_GMAC -DBCM47XX_CHOPS -DETROBO -DETADM -DBCMDMA64 -DBCMDMA32
endif

ifeq ($(CONFIG_BCM_FA),y)
et-objs += $(ETSRCDIR)/sys/etc_fa.o
EXTRA_CFLAGS    += -DETFA
endif

##
# Packet FastPath Module
##
#EXTRA_CFLAGS	+= -DCONFIG_PKTFP

##
# Add functions to verify skb offset w/CTF (remove ctf_srg_skb.o once new CTF arrives)
##
et-objs += $(ETSRCDIR)/sys/et_srg_skb.o

##
# Add functions to verify skb offset w/CTF (remove ctf_srg_skb.o once new CTF arrives)
##
et-objs += $(ETSRCDIR)/sys/et_srg_skb.o

obj-$(CONFIG_ET) := et.o

EXTRA_CFLAGS	+= -DDMA -DHAVE_NET_DEVICE_OPS -DET_INGRESS_QOS -Werror
ifeq ($(CONFIG_ET_ALL_PASSIVE_ON),y)
EXTRA_CFLAGS	+= -DET_ALL_PASSIVE_ON -DET_ALL_PASSIVE
else
ifeq ($(CONFIG_ET_ALL_PASSIVE_RUNTIME),y)
EXTRA_CFLAGS	+= -DET_ALL_PASSIVE
endif
endif	# /* CONFIG_ET_ALL_PASSIVE_ON */

# Search for sources under src/et/sys or objects under src/et/linux
ifneq ($(wildcard $(src)/$(ETSRCDIR)/sys),)
EXTRA_CFLAGS	+= -I$(src)/$(ETSRCDIR)/sys -I$(src)/$(ETSRCDIR)/include
else
et-objs := $(ETSRCDIR)/linux/et.o
endif

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
else
# Normal makefile
KERNELDIR := $(ETSRCDIR)/linux
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd`

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
endif

clean-files += $(ETSRCDIR)/sys/*.o $(ETSRCDIR)/sys/.*.o.cmd
