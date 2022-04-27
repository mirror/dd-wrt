#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer,
#    without modification.
# 2. Redistributions in binary form must reproduce at minimum a disclaimer
#    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
#    redistribution must be conditioned upon including a substantially
#    similar Disclaimer requirement for further binary redistribution.
# 3. Neither the names of the above-listed copyright holders nor the names
#    of any contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# Alternatively, this software may be distributed under the terms of the
# GNU General Public License ("GPL") version 2 as published by the Free
# Software Foundation.
#
# NO WARRANTY
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES.
#
# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/ath/Makefile#12 $
#

#
# Makefile for the Atheros WLAN driver.
#

ifeq ($(obj),)
obj=	.
endif

ifeq ($(strip $(BUS)),AHB)
BUSNAME=ahb
COPTS+= -DATH_AHB
else
BUSNAME=pci
COPTS+= -DATH_PCI
endif

# These flags select which radar signatures DFS will match
COPTS	+= -DDFS_DOMAIN_ETSI -DDFS_DOMAIN_FCC -Wno-unused

include $(TOP)/Makefile.inc

obj-m		+= ath_$(BUSNAME).o
ath_$(BUSNAME)-objs	:= if_ath.o if_ath_radar.o if_ath_$(BUSNAME).o
ifeq ($(CONFIG_LANTIQ),y)
ath_$(BUSNAME)-objs	+= if_ath_gpio_danube.o
else
ifeq ($(CONFIG_ARCH_IXP4XX),y)
ath_$(BUSNAME)-objs	+= if_ath_gpio_xscale.o
else
ifeq ($(CONFIG_ARCH_KS8695),y)
ath_$(BUSNAME)-objs	+= if_ath_gpio_ks8695.o
else
ifeq ($(CONFIG_MIPS_ADM5120),y)
ath_$(BUSNAME)-objs	+= if_ath_gpio_adm5120.o
else
ifeq ($(CONFIG_MACH_AR7100),y)
ath_$(BUSNAME)-objs	+= if_ath_gpio_ar71xx.o
else
ath_$(BUSNAME)-objs	+= if_ath_gpio_default.o
endif
endif
endif
endif
endif


ifneq ($(SINGLE_MODULE),)

include $(TOP)/net80211/Makefile
include $(TOP)/ath_rate/sample/Makefile
include $(TOP)/ath_rate/minstrel/Makefile
ath_$(BUSNAME)-objs += $(patsubst %,../net80211/%,$(wlan-objs) $(foreach var,wep tkip ccmp acl xauth scan_sta scan_ap,$(wlan_$(var)-objs))) ../ath_rate/sample/sample.o ../ath_rate/minstrel/minstrel.o
EXTRA_CFLAGS += -DSINGLE_MODULE 
endif

INCS += -I$(TOP) -I$(ATH_HAL) -I$(HAL) -I$(WLAN) -I.

EXTRA_CFLAGS+= $(INCS) $(COPTS) -DOPT_AH_H=\"public/$(TARGET).opt_ah.h\"

-include $(TOPDIR)/Rules.make

all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

install:
	test -d $(DESTDIR)/$(KMODPATH) || mkdir -p $(DESTDIR)/$(KMODPATH)
	install -m 0644 ath_$(BUSNAME).$(KMODSUF) $(DESTDIR)/$(KMODPATH)

ath_$(BUSNAME).o: $(ath_$(BUSNAME)-objs)
	cp *.o ../net80211
	cp *.o ../ath_rate/sample
	cp *.o ../ath_rate/minstrel
	$(LD) $(LDOPTS) -o ath_$(BUSNAME).$(KMODSUF) -r $(ath_$(BUSNAME)-objs)

if_ath_hal.h: $(HAL)/ah.h
	$(TOP)/scripts/if_ath_hal_generator.pl $< $@
	$(TOP)/scripts/madwifi-indent $@
