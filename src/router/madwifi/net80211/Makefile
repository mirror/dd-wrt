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
# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/net80211/Makefile#12 $
#

#
# Makefile for the 802.11 WLAN modules.
#
ifeq ($(obj),)
obj=	.
endif

DEPTH=	..
#
# There are two authenticator mechanisms: an in-kernel implementation
# (wlan_auth+wlan_radius) and an external implementation (wlan_xauth) that
# requires a user process to manage the authentication process.  By default
# the external authenticator is used.  ieee80211_proto.c has a table of module
# names that defines the default module to auto-load for each authentication
# scheme; to get the in-kernel authenticator by default modify it to load
# wlan_auth instead of wlan_xauth or manually load wlan_auth prior to use.
# 
MOD_AUTH	:= wlan_xauth.o
#
# Scanning policy is split into modules.  The default policy modules
# separate station-mode scanning support from ap-mode scanning support.
#
MOD_SCAN	:= wlan_scan_sta.o wlan_scan_ap.o
#
# Modules to be built+installed
#
MOD_INSTALL	:= wlan.o wlan_wep.o wlan_tkip.o wlan_ccmp.o wlan_acl.o \
		   $(MOD_AUTH) $(MOD_SCAN)

obj-m		+= $(MOD_INSTALL)

wlan-objs	:= if_media.o \
		   ieee80211.o ieee80211_beacon.o ieee80211_crypto.o \
		   ieee80211_crypto_none.o ieee80211_input.o ieee80211_node.o \
		   ieee80211_output.o ieee80211_power.o ieee80211_proto.o \
		   ieee80211_scan.o ieee80211_wireless.o ieee80211_linux.o \
		   ieee80211_monitor.o
wlan_wep-objs	:= ieee80211_crypto_wep.o
wlan_tkip-objs	:= ieee80211_crypto_tkip.o
wlan_ccmp-objs	:= ieee80211_crypto_ccmp.o
wlan_acl-objs	:= ieee80211_acl.o
wlan_xauth-objs	:= ieee80211_xauth.o
wlan_scan_sta-objs :=ieee80211_scan_sta.o
wlan_scan_ap-objs := ieee80211_scan_ap.o

include $(obj)/${DEPTH}/Makefile.inc

INCS+=	-I${HAL} -I${HAL}/${OS} -I$(obj)/${ATH_HAL} -I$(obj)/${WLAN} \
	-I${obj}/${DEPTH}

EXTRA_CFLAGS+=$(INCS) ${COPTS2}

-include $(TOPDIR)/Rules.make

all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

wlan.o:	$(wlan-objs)
	$(LD) $(LDOPTS) -o wlan.$(KMODSUF) -r $(wlan-objs)
wlan_wep.o:	$(wlan_wep-objs)
	$(LD) $(LDOPTS) -o wlan_wep.$(KMODSUF) -r $(wlan_wep-objs)
wlan_tkip.o:	$(wlan_tkip-objs)
	$(LD) $(LDOPTS) -o wlan_tkip.$(KMODSUF) -r $(wlan_tkip-objs)
wlan_ccmp.o:	$(wlan_ccmp-objs)
	$(LD) $(LDOPTS) -o wlan_ccmp.$(KMODSUF) -r $(wlan_ccmp-objs)
wlan_scan_sta.o: $(wlan_scan_sta-objs)
	$(LD) $(LDOPTS) -o wlan_scan_sta.$(KMODSUF) -r $(wlan_scan_sta-objs)
wlan_scan_ap.o:	$(wlan_scan_ap-objs)
	$(LD) $(LDOPTS) -o wlan_scan_ap.$(KMODSUF) -r $(wlan_scan_ap-objs)
wlan_xauth.o:	$(wlan_xauth-objs)
	$(LD) $(LDOPTS) -o wlan_xauth.$(KMODSUF) -r $(wlan_xauth-objs)
wlan_acl.o:	$(wlan_acl-objs)
	$(LD) $(LDOPTS) -o wlan_acl.$(KMODSUF) -r $(wlan_acl-objs)

install:
	test -d $(DESTDIR)/$(KMODPATH) || mkdir -p $(DESTDIR)/$(KMODPATH)
	for i in $(MOD_INSTALL); do \
		f=`basename $$i .o`; \
		cp $$f.$(KMODSUF) $(DESTDIR)/$(KMODPATH); \
	done

clean:
	-rm -f *~ *.o *.ko *.mod.c
	-rm -f .depend .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd
	-rm -rf .tmp_versions
