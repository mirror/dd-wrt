-include $(LINUXDIR)/.config
CONFIG_WR941=
CONFIG_WILLY=
CONFIG_WR941V6=
CONFIG_DHDAP=
CONFIG_EAP9550=
CONFIG_AP120C=
CONFIG_WA901=
CONFIG_WR650AC=
CONFIG_E355AC=
CONFIG_E380AC=
CONFIG_XD3200=
CONFIG_SR3200=
CONFIG_CPE890=
CONFIG_CPE880=
CONFIG_E325N=
CONFIG_XD9531=
CONFIG_WR615N=
CONFIG_MMS344=
CONFIG_DIR615I=
CONFIG_RB2011=
CONFIG_DAP3310=
CONFIG_WR741=
CONFIG_WZRAG300NH=
CONFIG_DIR615E=
CONFIG_WP543=
CONFIG_WDR4300=
CONFIG_WDR3500=
CONFIG_WDR2543=
CONFIG_JWAP606=
CONFIG_WRT160NL=
CONFIG_E2100L=
CONFIG_DIR825C1=
CONFIG_WZRG450NH=
CONFIG_WZRG450=
CONFIG_WZRG300NH2=
CONFIG_BUFFALO=
CONFIG_TPLINK=
CONFIG_DIR825=
CONFIG_WNDR3700=
CONFIG_WR1043V2=
CONFIG_WR841V9=
CONFIG_ARCHERC25=
CONFIG_WR841V10=
CONFIG_DAP2230=
CONFIG_DAP2662=
CONFIG_DAP2330=
CONFIG_DIR859=
CONFIG_DIR869=
CONFIG_GL150=
CONFIG_ERC=
CONFIG_CARAMBOLA=
CONFIG_FMS2111=

#
# Linux router shared code Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.10 2005/11/30 11:53:42 seg Exp $
#
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	= -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/
else
CFLAGS	= -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
endif
else
CFLAGS	= -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/
endif

ifeq ($(CONFIG_80211AC),y)
CFLAGS	= -I. -I$(TOP)/shared -I../httpd -Wall -I$(SRCBASE)/
CFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/include
CFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/common/include
CFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
CFLAGS += -I$(SRCBASE)/include.v24 
endif

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)
COPTS +=$(call cc-option,-Werror=return-local-addr,)


CFLAGS += $(MIPS16_OPT) -g3 $(THUMB)

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Iinclude
#  -Werror=maybe-uninitialized -Werror=int-conversion -Werror=array-bounds
CFLAGS	+= -fPIC
LDFLAGS += -L. -ldl
OBJS := libutils/utils.o libutils/ledconfig.o libutils/bridgetools.o libutils/detect.o libutils/linux_timer.o libutils/gpio.o libutils/md5c.o libutils/nvparse.o libutils/bcmutils.o libutils/linux_osl.o  libutils/ipfmt.o
SHOBJS =  libshutils/timezones.o libshutils/shutils.o libshutils/sha1.o libshutils/list.o libshutils/list_sort.o libshutils/airbag.o
WOBJS := libwireless/wl.o libwireless/wl_basic.o libwireless/wl_linux.o libwireless/country.o libwireless/wl_macfuncs.o

DISCOBJS   = libdisc/lib.o \
         libdisc/buffer.o libdisc/file.o libdisc/cdaccess.o libdisc/cdimage.o libdisc/vpc.o libdisc/compressed.o \
         libdisc/dos.o libdisc/cdrom.o libdisc/detect.o \
         libdisc/linux.o libdisc/unix.o libdisc/archives.o \
         libdisc/udf.o libdisc/blank.o libdisc/cloop.o libdisc/apple.o libdisc/beos.o libdisc/amiga.o libdisc/atari.o

ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
OBJS += libutils/libbridge_if.o
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
OBJS += libutils/libbridge_if.o
endif
endif

#defaults.o
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -fpic
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -fpic
endif
ifeq ($(CONFIG_DIST),"micro")
CFLAGS +=  -DHAVE_MICRO
CFLAGS += -DNEED_NOPRINTF
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS +=  -DHAVE_MICRO
CFLAGS += -DNEED_NOPRINTF
endif

ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
LDFLAGS += -lpthread
endif
endif

ifeq ($(CONFIG_SUPERCHANNEL),y)
CFLAGS += -DHAVE_SUPERCHANNEL
ATH9KCFLAGS += -DHAVE_SUPERCHANNEL
SHOBJS += $(TOP)/register/register_check.o
OBJS += $(TOP)/register/register_regulatory.o
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
ATH9KCFLAGS += -DHAVE_REGISTER
SHOBJS += $(TOP)/register/register_check.o
OBJS += $(TOP)/register/register_regulatory.o
endif
ifeq ($(CONFIG_MADWIFI),y)
SHOBJS += $(TOP)/register/register_check.o
WOBJS += $(TOP)/register/register_wifi.o
OBJS += $(TOP)/register/register_regulatory.o
endif
ifeq ($(CONFIG_IPV6),y)
#OBJS += libutils/getifaddrs.o
CFLAGS += -DHAVE_IPV6
endif
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
  CFLAGS += -DHAVE_DROPBEAR -DHAVE_SSHD
endif
ifeq ($(CONFIG_HOTPLUG2),y)
CFLAGS += -DHAVE_HOTPLUG2
endif
ifeq ($(CONFIG_OPENDPI),y)
CFLAGS += -DHAVE_OPENDPI
endif
ifeq ($(CONFIG_ATH9K),y)
CFLAGS += -DHAVE_ATH9K
endif
ifeq ($(CONFIG_ATH5K),y)
CFLAGS += -DHAVE_ATH5K
endif
ifeq ($(CONFIG_ATH10K),y)
CFLAGS += -DHAVE_ATH10K
endif
ifeq ($(CONFIG_ATH11K),y)
CFLAGS += -DHAVE_ATH11K
endif
ifeq ($(CONFIG_IPQ6018),y)
CFLAGS += -DHAVE_ATH11K
endif
ifeq ($(CONFIG_BRCMFMAC),y)
CFLAGS += -DHAVE_BRCMFMAC
endif
ifeq ($(CONFIG_MT76),y)
CFLAGS += -DHAVE_MT76
endif
ifeq ($(CONFIG_ATH9K),y)
ATH9KCFLAGS+= $(COPTS) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -DHAVE_ATH9K -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v24 -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/libnl-tiny/include -I$(SRCBASE)/
ATH9KCFLAGS	+= -g3 -fPIC $(call cc-option,-Werror=return-local-addr)
ATH9KOBJS += libwireless/wl_ath9k.o libwireless/mac80211info.o libwireless/mac80211regulatory.o libwireless/mac80211autochannel.o
ATH9KLDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
ATH9KCFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/include
ATH9KCFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
ifeq ($(CONFIG_MAC80211_MESH),y)
ATH9KCFLAGS += -DHAVE_MAC80211_MESH
endif


ifeq ($(CONFIG_SYSLOG),y)
ATH9KCFLAGS += -DHAVE_SYSLOG
endif
ifeq ($(CONFIG_ATH5K),y)
ATH9KCFLAGS += -DHAVE_ATH5K
endif
ifeq ($(CONFIG_BRCMFMAC),y)
ATH9KCFLAGS += -DHAVE_BRCMFMAC
endif
ifeq ($(CONFIG_MT76),y)
ATH9KCFLAGS += -DHAVE_MT76
endif
ifeq ($(CONFIG_DIST),"micro")
ATH9KCFLAGS +=  -DHAVE_MICRO
ATH9KCFLAGS += -DNEED_NOPRINTF
endif
ifeq ($(CONFIG_DIST),"micro-special")
ATH9KCFLAGS +=  -DHAVE_MICRO
ATH9KCFLAGS += -DNEED_NOPRINTF
endif
endif

ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ATH9KLDFLAGS += -lpthread
endif
endif



ifeq ($(CONFIG_ATH10K),y)
ATH9KCFLAGS += -DHAVE_ATH10K
endif
ifeq ($(CONFIG_ATH11K),y)
ATH9KCFLAGS += -DHAVE_ATH11K
endif
ifeq ($(CONFIG_IPQ6018),y)
ATH9KCFLAGS += -DHAVE_ATH11K
ATH9KCFLAGS += -DHAVE_IPQ6018
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI -I../services/networking/wifi -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL 
WLDFLAGS = -L $(TOP)/wireless-tools -liw
else
#WLDFLAGS = -lutils
endif


ifeq ($(CONFIG_SILENCE),y)
  CFLAGS += -DHAVE_SILENCE
endif
ifeq ($(CONFIG_ALLNETWRT),y)
  CFLAGS += -DHAVE_ALLNETWRT
endif
ifeq ($(CONFIG_ALLNET11N),y)
  CFLAGS += -DHAVE_ALLNET11N
endif
ifeq ($(CONFIG_BUFFALO),y)
  CFLAGS += -DHAVE_BUFFALO
  NCFLAGS += -DHAVE_BUFFALO
  ATH9KCFLAGS += -DHAVE_BUFFALO
endif
ifeq ($(CONFIG_BUFFALO_SA),y)
  CFLAGS += -DHAVE_BUFFALO_SA
  NCFLAGS += -DHAVE_BUFFALO_SA
  ATH9KCFLAGS += -DHAVE_BUFFALO_SA
endif
ifeq ($(CONFIG_BUFFALO_BL_DEFAULTS),y)
  CFLAGS += -DHAVE_BUFFALO_BL_DEFAULTS
endif
ifeq ($(CONFIG_LAGUNA),y)
  CFLAGS += -DHAVE_LAGUNA
endif
ifeq ($(CONFIG_VENTANA),y)
  CFLAGS += -DHAVE_VENTANA
endif
ifeq ($(CONFIG_NEWPORT),y)
  CFLAGS += -DHAVE_NEWPORT
endif
ifeq ($(CONFIG_NORTHSTAR),y)
  CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_CARAMBOLA),y)
  CFLAGS += -DHAVE_CARAMBOLA
  ifeq ($(CONFIG_FMS2111),y)
    CFLAGS += -DHAVE_FMS2111
  endif
endif
ifeq ($(CONFIG_ANTAIRA),y)
    CFLAGS += -DHAVE_ANTAIRA
endif
ifeq ($(CONFIG_ANTAIRA_MINI),y)
    CFLAGS += -DHAVE_ANTAIRA_MINI
endif
#ifeq ($(CONFIG_SYSCTL_EDIT),y)
    CFLAGS += -DHAVE_SYSCTL_EDIT
#endif

ifeq ($(CONFIG_NEXTMEDIA),y)
  CFLAGS += -DHAVE_NEXTMEDIA
endif
ifeq ($(CONFIG_ALL02310N),y)
  CFLAGS += -DHAVE_ALL02310N
endif
ifeq ($(CONFIG_RAIEXTRA),y)
CFLAGS += -DHAVE_RAIEXTRA
endif
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
  CFLAGS += -DBUFFALO_JP
endif
ifeq ($(CONFIG_SAMBA),y)
  CFLAGS += -DHAVE_SAMBA
endif
ifeq ($(CONFIG_QTN),y)
  CFLAGS += -DHAVE_QTN -DRPCOVERRIDE
  WOBJS += libwireless/qtnapi.o 
  CFLAGS +=  -I$(TOP)/qtn/libqcsapi_client -I$(TOP)/qtn/libqcsapi_client/librpc 
  QTNFLAGS =  -L$(TOP)/qtn/libqcsapi_client -lqcsapi_client   -L$(TOP)/qtn/libqcsapi_client/librpc -lrpc
endif
ifeq ($(CONFIG_TIEXTRA1),y)
  OBJS += $(TOP)/private/telkom/mchillispot.o
  CFLAGS += -I$(TOP)/jansson/src -I$(TOP)/private/telkom -L$(TOP)/jansson/src/.libs -ljansson -lm
  CFLAGS += -DHAVE_TIEXTRA1
endif
ifeq ($(CONFIG_TIEXTRA2),y)
  OBJS += $(TOP)/private/telkom/mwifidog.o
  CFLAGS += -I$(TOP)/jansson/src -I$(TOP)/private/telkom -L$(TOP)/jansson/src/.libs -ljansson -lm
  CFLAGS += -DHAVE_TIEXTRA2
endif
ifeq ($(CONFIG_GEMTEK),y)
  CFLAGS += -DHAVE_GEMTEK
endif
ifeq ($(CONFIG_WAVESAT),y)
  CFLAGS += -DHAVE_WAVESAT
endif
ifeq ($(CONFIG_GLAUCO),y)
CFLAGS += -DHAVE_GLAUCO
endif
ifeq ($(CONFIG_WR650AC),y)
CFLAGS += -DHAVE_WR650AC
endif
ifeq ($(CONFIG_WPA3),y)
ifeq ($(HOSTAPDVERSION),)
CFLAGS += -DHAVE_WPA3 -DHAVE_80211R -DHAVE_80211W
endif
ifeq ($(HOSTAPDVERSION),2023-09-08)
CFLAGS += -DHAVE_WPA3 -DHAVE_80211R -DHAVE_80211W
endif
ifeq ($(HOSTAPDVERSION),2018-07-08)
CFLAGS += -DHAVE_WPA3 -DHAVE_80211R -DHAVE_80211W
endif
endif
ifeq ($(CONFIG_E355AC),y)
CFLAGS += -DHAVE_E355AC
endif
ifeq ($(CONFIG_WILLY),y)
CFLAGS += -DHAVE_WILLY
endif
ifeq ($(CONFIG_MSTP),y)
CFLAGS += -DHAVE_MSTP
endif
ifeq ($(CONFIG_E380AC),y)
CFLAGS += -DHAVE_E380AC
endif
ifeq ($(CONFIG_XD3200),y)
CFLAGS += -DHAVE_XD3200
endif
ifeq ($(CONFIG_SR3200),y)
CFLAGS += -DHAVE_SR3200
endif
ifeq ($(CONFIG_CPE890),y)
CFLAGS += -DHAVE_CPE890
endif
ifeq ($(CONFIG_CPE880),y)
CFLAGS += -DHAVE_CPE880
endif
ifeq ($(CONFIG_AP120C),y)
CFLAGS += -DHAVE_AP120C
endif
ifeq ($(CONFIG_E325N),y)
CFLAGS += -DHAVE_E325N
endif
ifeq ($(CONFIG_XD9531),y)
CFLAGS += -DHAVE_XD9531
endif
ifeq ($(CONFIG_WR615N),y)
CFLAGS += -DHAVE_WR615N
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_VLANTAGGING
endif

ifeq ($(CONFIG_RAMSKOV),y)
  CFLAGS += -DHAVE_RAMSKOV
endif

ifeq ($(CONFIG_SNMP),y)
  CFLAGS += -DHAVE_SNMP
endif

ifeq ($(CONFIG_AC622),y)
  CFLAGS += -DHAVE_AC622
endif

ifeq ($(CONFIG_JJAP005),y)
  CFLAGS += -DHAVE_JJAP005
endif
ifeq ($(CONFIG_JJAP501),y)
  CFLAGS += -DHAVE_JJAP501
endif
ifeq ($(CONFIG_AC722),y)
  CFLAGS += -DHAVE_AC722
endif
ifeq ($(CONFIG_UNIWIP),y)
  CFLAGS += -DHAVE_UNIWIP
endif

ifeq ($(REGDOMAIN_OVERRIDE),y)
CFLAGS += -DREGDOMAIN_OVERRIDE
endif

ifeq ($(CONFIG_RB500),y)
  CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_WHR300HP2),y)
  CFLAGS += -DHAVE_WHR300HP2
endif
ifeq ($(CONFIG_E1700),y)
  CFLAGS += -DHAVE_E1700
endif
ifeq ($(CONFIG_WRT1900AC),y)
  CFLAGS += -DHAVE_WRT1900AC
endif
ifeq ($(CONFIG_MVEBU),y)
  CFLAGS += -DHAVE_MVEBU
  ATH9KCFLAGS += -DHAVE_MVEBU
endif
ifeq ($(CONFIG_IPQ806X),y)
  CFLAGS += -DHAVE_IPQ806X
endif
ifeq ($(CONFIG_IPQ6018),y)
  CFLAGS += -DHAVE_IPQ6018
endif
ifeq ($(CONFIG_WIL6210),y)
  CFLAGS += -DHAVE_WIL6210
  ATH9KCFLAGS += -DHAVE_WIL6210
endif
ifeq ($(CONFIG_R7500),y)
  CFLAGS += -DHAVE_R7500
endif
ifeq ($(CONFIG_MT7620),y)
  CFLAGS += -DHAVE_MT7620
endif
ifeq ($(CONFIG_WR941NV6_CN),y)
  CFLAGS += -DHAVE_WR941NV6_CN
endif
ifeq ($(CONFIG_MT7621),y)
  CFLAGS += -DHAVE_MT7621
endif

ifeq ($(CONFIG_XSCALE),y)
  CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_VF803),y)
  CFLAGS += -DHAVE_VF803
endif
ifeq ($(CONFIG_SWCONFIG),y)
  CFLAGS += -DHAVE_SWCONFIG
endif
ifeq ($(CONFIG_SX763),y)
  CFLAGS += -DHAVE_SX763
endif
ifeq ($(CONFIG_WR1043V4),y)
  CFLAGS += -DHAVE_WR1043V4
endif
ifeq ($(CONFIG_WR1043V5),y)
  CFLAGS += -DHAVE_WR1043V5
endif
ifeq ($(CONFIG_LIMA),y)
  CFLAGS += -DHAVE_LIMA
endif
ifeq ($(CONFIG_PERU),y)
  CFLAGS += -DHAVE_PERU
endif
ifeq ($(CONFIG_RAMBUTAN),y)
  CFLAGS += -DHAVE_RAMBUTAN
endif
ifeq ($(CONFIG_DW02_412H),y)
  CFLAGS += -DHAVE_DW02_412H
endif
ifeq ($(CONFIG_VF802),y)
  CFLAGS += -DHAVE_VF802
endif
ifeq ($(CONFIG_CAMBRIA),y)
  CFLAGS += -DHAVE_CAMBRIA
endif
ifeq ($(CONFIG_GATEWORX),y)
  CFLAGS += -DHAVE_GATEWORX
endif
ifeq ($(CONFIG_PRONGHORN),y)
  CFLAGS += -DHAVE_PRONGHORN
endif
ifeq ($(CONFIG_USR8200),y)
  CFLAGS += -DHAVE_USR8200
endif
ifeq ($(CONFIG_WG302),y)
  CFLAGS += -DHAVE_WG302
endif
ifeq ($(CONFIG_DLM101),y)
  CFLAGS += -DHAVE_DLM101
endif
ifeq ($(CONFIG_OCTEON),y)
  CFLAGS += -DHAVE_OCTEON
endif
ifeq ($(CONFIG_EROUTER),y)
  CFLAGS += -DHAVE_EROUTER
endif
ifeq ($(CONFIG_DHDAP),y)
  CFLAGS += -DHAVE_DHDAP
endif
ifeq ($(CONFIG_E200),y)
  CFLAGS += -DHAVE_E200
endif
ifeq ($(CONFIG_ZFS),y)
  CFLAGS += -DHAVE_ZFS
endif
ifeq ($(CONFIG_RAID),y)
  CFLAGS += -DHAVE_RAID
endif
ifeq ($(CONFIG_RUT500),y)
  CFLAGS += -DHAVE_RUT500
endif
ifeq ($(CONFIG_DIR859),y)
  CFLAGS += -DHAVE_DIR859
endif
ifeq ($(CONFIG_DIR869),y)
  CFLAGS += -DHAVE_DIR869
endif
ifeq ($(CONFIG_DIR866),y)
  CFLAGS += -DHAVE_DIR866
endif
ifeq ($(CONFIG_DIR862),y)
  CFLAGS += -DHAVE_DIR862
endif
ifeq ($(CONFIG_DAP3662),y)
  CFLAGS += -DHAVE_DAP3662
endif
ifeq ($(CONFIG_WG302V1),y)
  CFLAGS += -DHAVE_WG302V1
endif
ifeq ($(CONFIG_ALFA_BRANDING2),y)
  CFLAGS += -DHAVE_ALFA_BRANDING
endif
ifeq ($(CONFIG_ALFA_BRANDING1),y)
  CFLAGS += -DHAVE_ALFA_BRANDING
endif

ifeq ($(CONFIG_NOP8670),y)
  CFLAGS += -DHAVE_NOP8670
endif
ifeq ($(CONFIG_DIR600),y)
  CFLAGS += -DHAVE_DIR600
endif
ifeq ($(CONFIG_RT10N),y)
  CFLAGS += -DHAVE_RT10N
endif
ifeq ($(CONFIG_RB2011),y)
  CFLAGS += -DHAVE_RB2011
endif
ifeq ($(CONFIG_DIR825C1),y)
  CFLAGS += -DHAVE_DIR825C1
endif
ifeq ($(CONFIG_DHP1565),y)
  CFLAGS += -DHAVE_DHP1565
endif
ifeq ($(CONFIG_DIR835A1),y)
  CFLAGS += -DHAVE_DIR835A1
endif
ifeq ($(CONFIG_WDR4300),y)
  CFLAGS += -DHAVE_WDR4300
endif
ifeq ($(CONFIG_WNDR4300),y)
  CFLAGS += -DHAVE_WNDR4300
endif
ifeq ($(CONFIG_WDR3600),y)
  CFLAGS += -DHAVE_WDR3600
endif
ifeq ($(CONFIG_WDR3500),y)
  CFLAGS += -DHAVE_WDR3500
endif
ifeq ($(CONFIG_WDR2543),y)
  CFLAGS += -DHAVE_WDR2543
endif

ifeq ($(CONFIG_TECHNAXX),y)
  CFLAGS += -DHAVE_TECHNAXX
endif
ifeq ($(CONFIG_BUFFALO_SA),y)
  CFLAGS += -DHAVE_BUFFALO_SA
endif
ifeq ($(CONFIG_NEPTUNE),y)
  CFLAGS += -DHAVE_NEPTUNE
endif
ifeq ($(CONFIG_R9000),y)
  CFLAGS += -DHAVE_R9000
endif
ifeq ($(CONFIG_ALPINE),y)
  CFLAGS += -DHAVE_ALPINE
endif
ifeq ($(CONFIG_WR841HPV3),y)
  CFLAGS += -DHAVE_WR841HPV3
endif
ifeq ($(CONFIG_RT3352),y)
  CFLAGS += -DHAVE_RT3352
endif
ifeq ($(CONFIG_CORENET),y)
  CFLAGS += -DHAVE_CORENET
endif
ifeq ($(CONFIG_XIOCOM),y)
  CFLAGS += -DHAVE_XIOCOM
endif
ifeq ($(CONFIG_OLED),y)
CFLAGS += -DHAVE_OLED
endif
ifeq ($(CONFIG_WRT160NL),y)
CFLAGS += -DHAVE_WRT160NL
endif
ifeq ($(CONFIG_E2100),y)
CFLAGS += -DHAVE_E2100
endif
ifeq ($(CONFIG_ECB9750),y)
  CFLAGS += -DHAVE_ECB9750
endif
ifeq ($(CONFIG_TECHNAXX3G),y)
  CFLAGS += -DHAVE_TECHNAXX3G
endif
ifeq ($(CONFIG_AR670W),y)
  CFLAGS += -DHAVE_AR670W
endif
ifeq ($(CONFIG_AR690W),y)
  CFLAGS += -DHAVE_AR690W
endif
ifeq ($(CONFIG_RT15N),y)
  CFLAGS += -DHAVE_RT15N
endif
ifeq ($(CONFIG_BR6574N),y)
  CFLAGS += -DHAVE_BR6574N
endif
ifeq ($(CONFIG_RT2880),y)
  CFLAGS += -DHAVE_RT2880 -I../wireless-tools -DHEADERS_KERNEL
endif
ifeq ($(CONFIG_RT61),y)
  CFLAGS += -DHAVE_RT61 -I../wireless-tools -DHEADERS_KERNEL
endif
ifeq ($(CONFIG_ESR9752),y)
CFLAGS += -DHAVE_ESR9752
endif
ifeq ($(CONFIG_ACXNR22),y)
CFLAGS += -DHAVE_ACXNR22
endif
ifeq ($(CONFIG_W502U),y)
CFLAGS += -DHAVE_W502U
endif
ifeq ($(CONFIG_F5D8235),y)
CFLAGS += -DHAVE_F5D8235
endif
ifeq ($(CONFIG_MI424WR),y)
CFLAGS += -DHAVE_MI424WR
endif
ifeq ($(CONFIG_WHRG300N),y)
  CFLAGS += -DHAVE_WHRG300N
endif
ifeq ($(CONFIG_HAMEA15),y)
  CFLAGS += -DHAVE_HAMEA15
endif
ifeq ($(CONFIG_IWLWIFI),y)
  CFLAGS += -DHAVE_IWLWIFI
endif
ifeq ($(CONFIG_WCRGN),y)
  CFLAGS += -DHAVE_WCRGN
endif
ifeq ($(CONFIG_WR5422),y)
  CFLAGS += -DHAVE_WR5422
endif
ifeq ($(CONFIG_ESR6650),y)
  CFLAGS += -DHAVE_ESR6650
endif
ifeq ($(CONFIG_EAP9550),y)
  CFLAGS += -DHAVE_EAP9550
endif
ifeq ($(CONFIG_X86),y)
  CFLAGS += -DHAVE_X86
endif
ifeq ($(CONFIG_X64),y)
  CFLAGS += -DHAVE_X64
endif
ifeq ($(CONFIG_3G),y)
  CFLAGS += -DHAVE_3G
  OBJS += libutils/lib3g.o
endif
ifeq ($(CONFIG_UQMI),y)
  CFLAGS += -DHAVE_UQMI
endif
ifeq ($(CONFIG_LIBQMI),y)
  CFLAGS += -DHAVE_LIBQMI
endif
ifeq ($(CONFIG_MAGICBOX),y)
  CFLAGS += -DHAVE_MAGICBOX
endif
ifeq ($(CONFIG_RB600),y)
  CFLAGS += -DHAVE_RB600
endif
ifeq ($(CONFIG_WDR4900),y)
  CFLAGS += -DHAVE_WDR4900
endif
ifeq ($(CONFIG_WDR4900V2),y)
  CFLAGS += -DHAVE_WDR4900V2
endif
ifeq ($(CONFIG_RB800),y)
  CFLAGS += -DHAVE_RB800
endif
ifeq ($(CONFIG_RB1000),y)
  CFLAGS += -DHAVE_RB1000
endif
ifeq ($(CONFIG_FREERADIUS),y)
  CFLAGS += -DHAVE_FREERADIUS
  OBJS += libutils/radius.o
endif
ifeq ($(CONFIG_POKER),y)
  CFLAGS += -DHAVE_POKER
  OBJS += libutils/poker.o
endif
ifeq ($(CONFIG_HOBBIT),y)
  CFLAGS += -DHAVE_HOBBIT
endif
ifeq ($(CONFIG_ACK),y)
  CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_MR3202A),y)
  CFLAGS += -DHAVE_MR3202A
  CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_MR3201A),y)
  CFLAGS += -DHAVE_MR3201A
  CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_WRK54G),y)
  CFLAGS += -DHAVE_WRK54G
endif
ifeq ($(CONFIG_WRT160NL),y)
CFLAGS += -DHAVE_WRT160NL
endif
ifeq ($(CONFIG_WZRG300NH),y)
  CFLAGS += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS += -DHAVE_WZRHPAG300NH
ATH9KCFLAGS += -DHAVE_WZRHPAG300NH
endif
ifeq ($(CONFIG_WZR600DHP),y)
CFLAGS += -DHAVE_WZR600DHP
endif
ifeq ($(CONFIG_M400XW),y)
CFLAGS += -DHAVE_M400XW
endif
ifeq ($(CONFIG_WHR1166D),y)
CFLAGS += -DHAVE_WHR1166D
endif
ifeq ($(CONFIG_DIR810L),y)
CFLAGS += -DHAVE_DIR810L
endif
ifeq ($(CONFIG_DIR860),y)
CFLAGS += -DHAVE_DIR860
endif
ifeq ($(CONFIG_R6800),y)
CFLAGS += -DHAVE_R6800
endif
ifeq ($(CONFIG_MT7612),y)
CFLAGS += -DHAVE_MT7612
endif
ifeq ($(CONFIG_DIR825),y)
  CFLAGS += -DHAVE_DIR825
endif
ifeq ($(CONFIG_TEW673GRU),y)
  CFLAGS += -DHAVE_TEW673GRU
endif
ifeq ($(CONFIG_WNDR3700),y)
  CFLAGS += -DHAVE_WNDR3700
endif
ifeq ($(CONFIG_WNDR3700V2),y)
  CFLAGS += -DHAVE_WNDR3700V2
endif
ifeq ($(CONFIG_WNDR3700V4),y)
  CFLAGS += -DHAVE_WNDR3700V4
endif
ifeq ($(CONFIG_WMBR_G300NH),y)
CFLAGS += -DHAVE_WMBR_G300NH
endif

ifeq ($(CONFIG_WRT400),y)
  CFLAGS += -DHAVE_WRT400
endif
ifeq ($(CONFIG_SYSLOG),y)
  CFLAGS += -DHAVE_SYSLOG
endif
ifeq ($(CONFIG_TONZE),y)
  CFLAGS += -DHAVE_TONZE
endif
ifeq ($(CONFIG_WRT54GV7),y)
  CFLAGS += -DHAVE_WRT54GV7
endif
ifeq ($(CONFIG_AR430W),y)
  CFLAGS += -DHAVE_AR430W
endif
ifeq ($(CONFIG_WRT300NV2),y)
  CFLAGS += -DHAVE_WRT300NV2
endif
ifeq ($(CONFIG_WRT54G2),y)
  CFLAGS += -DHAVE_WRT54G2
endif
ifeq ($(CONFIG_RTG32),y)
  CFLAGS += -DHAVE_RTG32
endif
ifeq ($(CONFIG_DIR400),y)
  CFLAGS += -DHAVE_DIR400
  CFLAGS += -DHAVE_FONERA
else
ifeq ($(CONFIG_DIR300),y)
  CFLAGS += -DHAVE_DIR300
  CFLAGS += -DHAVE_FONERA
endif
endif
ifeq ($(CONFIG_BCMMODERN),y)
  CFLAGS += -DHAVE_BCMMODERN
endif
ifeq ($(CONFIG_80211AC),y)
  CFLAGS += -DHAVE_80211AC
endif
ifeq ($(CONFIG_FONERA),y)
  CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_WBD500),y)
  CFLAGS += -DHAVE_WBD500
endif
ifeq ($(CONFIG_TRIMAX),y)
  CFLAGS += -DHAVE_TRIMAX
endif
ifeq ($(CONFIG_WIKINGS),y)
  CFLAGS += -DHAVE_WIKINGS
endif
ifeq ($(CONFIG_ESPOD),y)
  CFLAGS += -DHAVE_ESPOD
endif
ifeq ($(CONFIG_CARLSONWIRELESS),y)
  CFLAGS += -DHAVE_CARLSONWIRELESS
endif
ifeq ($(CONFIG_IMMERSIVE),y)
  CFLAGS += -DHAVE_IMMERSIVE
endif
ifeq ($(CONFIG_HDWIFI),y)
  CFLAGS += -DHAVE_HDWIFI
endif
ifeq ($(CONFIG_IPR),y)
  CFLAGS += -DHAVE_IPR
endif
ifeq ($(CONFIG_ONNET),y)
  CFLAGS += -DHAVE_ONNET
endif
ifeq ($(CONFIG_RAYTRONIK),y)
  CFLAGS += -DHAVE_RAYTRONIK
endif
ifeq ($(CONFIG_IDEXX),y)
  CFLAGS += -DHAVE_IDEXX
  ATH9KCFLAGS += -DHAVE_IDEXX
  ifeq ($(CONFIG_IDEXX_WORLD),y)
    ATH9KCFLAGS += -DHAVE_IDEXX_WORLD
  endif
endif
ifeq ($(CONFIG_SUB3),y)
CFLAGS += -DHAVE_SUB3
endif
ifeq ($(CONFIG_SUB6),y)
CFLAGS += -DHAVE_SUB6
endif
ifeq ($(CONFIG_MYNETWAY),y)
  CFLAGS += -DHAVE_MYNETWAY
endif
ifeq ($(CONFIG_WGT624),y)
  CFLAGS += -DHAVE_WGT624
endif
ifeq ($(CONFIG_NP25G),y)
  CFLAGS += -DHAVE_NP25G
endif
ifeq ($(CONFIG_WPE53G),y)
  CFLAGS += -DHAVE_WPE53G
endif
ifeq ($(CONFIG_SOLO51),y)
  CFLAGS += -DHAVE_SOLO51
endif
ifeq ($(CONFIG_MERAKI),y)
  CFLAGS += -DHAVE_MERAKI
endif
ifeq ($(CONFIG_LS2),y)
  CFLAGS += -DHAVE_LS2
endif
ifeq ($(CONFIG_WPE72),y)
  CFLAGS += -DHAVE_WPE72
endif
ifeq ($(CONFIG_WP543),y)
  CFLAGS += -DHAVE_WP543
endif
ifeq ($(CONFIG_WP546),y)
  CFLAGS += -DHAVE_WP546
endif
CFLAGS += -DARCH_$(ARCHITECTURE)
ifeq ($(CONFIG_BWRG1000),y)
  CFLAGS += -DHAVE_BWRG1000
endif
ifeq ($(CONFIG_ERC),y)
CFLAGS += -DHAVE_ERC
endif
ifeq ($(CONFIG_WHA5500CPE),y)
  CFLAGS += -DHAVE_WHA5500CPE
endif
ifeq ($(CONFIG_AIRMAX5),y)
  CFLAGS += -DHAVE_AIRMAX5
endif
ifeq ($(CONFIG_ECB3500),y)
  CFLAGS += -DHAVE_ECB3500
endif
ifeq ($(CONFIG_DDLINK),y)
  CFLAGS += -DHAVE_DDLINK
endif
ifeq ($(CONFIG_EAP3660),y)
  CFLAGS += -DHAVE_EAP3660
endif
ifeq ($(CONFIG_ASUSRTN13U),y)
  CFLAGS += -DHAVE_ASUSRTN13U
endif
ifeq ($(CONFIG_RT13NB1),y)
  CFLAGS += -DHAVE_RT13NB1
endif
ifeq ($(CONFIG_EOC2610),y)
  CFLAGS += -DHAVE_EOC2610
endif
ifeq ($(CONFIG_EOC2611),y)
  CFLAGS += -DHAVE_EOC2611
endif
ifeq ($(CONFIG_EOC1650),y)
  CFLAGS += -DHAVE_EOC1650
endif
ifeq ($(CONFIG_NS2),y)
  CFLAGS += -DHAVE_NS2
endif
ifeq ($(CONFIG_R2M),y)
  CFLAGS += -DHAVE_R2M
endif
ifeq ($(CONFIG_R5M),y)
  CFLAGS += -DHAVE_R5M
endif
ifeq ($(CONFIG_BS2MHP),y)
  CFLAGS += -DHAVE_BS2MHP
endif
ifeq ($(CONFIG_BS5MHP),y)
  CFLAGS += -DHAVE_BS5MHP
endif
ifeq ($(CONFIG_NS2M),y)
  CFLAGS += -DHAVE_NS2M
endif
ifeq ($(CONFIG_NS5M),y)
  CFLAGS += -DHAVE_NS5M
endif
ifeq ($(CONFIG_NS5),y)
  CFLAGS += -DHAVE_NS5
endif
ifeq ($(CONFIG_EOC5610),y)
  CFLAGS += -DHAVE_EOC5610
endif
ifeq ($(CONFIG_EOC5510),y)
  CFLAGS += -DHAVE_EOC5510
endif
ifeq ($(CONFIG_EOC5611),y)
  CFLAGS += -DHAVE_EOC5611
endif
ifeq ($(CONFIG_NS3),y)
  CFLAGS += -DHAVE_NS3
endif
ifeq ($(CONFIG_BS5),y)
  CFLAGS += -DHAVE_BS5
endif
ifeq ($(CONFIG_BS2),y)
  CFLAGS += -DHAVE_BS2
endif
ifeq ($(CONFIG_PICO2),y)
  CFLAGS += -DHAVE_PICO2
endif
ifeq ($(CONFIG_PICO2HP),y)
  CFLAGS += -DHAVE_PICO2HP
endif
ifeq ($(CONFIG_PICO5),y)
  CFLAGS += -DHAVE_PICO5
endif
ifeq ($(CONFIG_MS2),y)
  CFLAGS += -DHAVE_MS2
endif
ifeq ($(CONFIG_BS2HP),y)
  CFLAGS += -DHAVE_BS2HP
endif
ifeq ($(CONFIG_LC2),y)
  CFLAGS += -DHAVE_LC2
endif
ifeq ($(CONFIG_LC5),y)
  CFLAGS += -DHAVE_LC5
endif
ifeq ($(CONFIG_PS2),y)
  CFLAGS += -DHAVE_PS2
endif
ifeq ($(CONFIG_PS5),y)
  CFLAGS += -DHAVE_PS5
endif
ifeq ($(CONFIG_WP54G),y)
  CFLAGS += -DHAVE_WP54G
endif
ifeq ($(CONFIG_NP28G),y)
  CFLAGS += -DHAVE_NP28G
endif
ifeq ($(CONFIG_ADM5120),y)
  CFLAGS += -DHAVE_ADM5120
endif
ifeq ($(CONFIG_LS5),y)
  CFLAGS += -DHAVE_LS5
endif
ifeq ($(CONFIG_WHRAG108),y)
  CFLAGS += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_PB42),y)
  CFLAGS += -DHAVE_PB42
endif
ifeq ($(CONFIG_RS),y)
  CFLAGS += -DHAVE_RS
endif
ifeq ($(CONFIG_RSPRO),y)
  CFLAGS += -DHAVE_RSPRO
endif
ifeq ($(CONFIG_JA76PF),y)
  CFLAGS += -DHAVE_JA76PF
endif
ifeq ($(CONFIG_ALFAAP94),y)
  CFLAGS += -DHAVE_ALFAAP94
endif
ifeq ($(CONFIG_JWAP003),y)
  CFLAGS += -DHAVE_JWAP003
endif
ifeq ($(CONFIG_JWAP606),y)
  CFLAGS += -DHAVE_JWAP606
endif
ifeq ($(CONFIG_AP83),y)
  CFLAGS += -DHAVE_AP83
endif
ifeq ($(CONFIG_LSX),y)
  CFLAGS += -DHAVE_LSX
endif
ifeq ($(CONFIG_DANUBE),y)
  CFLAGS += -DHAVE_DANUBE
endif
ifeq ($(CONFIG_STORM),y)
  CFLAGS += -DHAVE_STORM
endif
ifeq ($(CONFIG_WBD222),y)
  CFLAGS += -DHAVE_WBD222
endif
ifeq ($(CONFIG_OPENRISC),y)
  CFLAGS += -DHAVE_OPENRISC
endif
ifeq ($(CONFIG_TW6600),y)
  CFLAGS += -DHAVE_TW6600
endif
ifeq ($(CONFIG_CA8),y)
  CFLAGS += -DHAVE_CA8
endif
ifeq ($(CONFIG_PPPOE),y)
  CFLAGS += -DHAVE_PPPOE
  CFLAGS += -DHAVE_PPPOEDUAL
endif
ifeq ($(CONFIG_PPPOATM),y)
  CFLAGS += -DHAVE_PPPOATM
endif
ifeq ($(CONFIG_PPTP),y)
  CFLAGS += -DHAVE_PPPOA
endif
ifeq ($(CONFIG_L2TP),y)
  CFLAGS += -DHAVE_L2TP
endif
ifeq ($(CONFIG_CA8PRO),y)
  CFLAGS += -DHAVE_CA8PRO
endif
ifeq ($(CONFIG_RCAA01),y)
  CFLAGS += -DHAVE_RCAA01
endif
ifeq ($(CONFIG_RDAT81),y)
  CFLAGS += -DHAVE_RDAT81
endif

ifeq ($(CONFIG_USR5453),y)
  CFLAGS += -DHAVE_USR5453
endif

ifeq ($(CONFIG_PPPOERELAY),y)
  CFLAGS += -DHAVE_PPPOERELAY
endif
ifeq ($(CONFIG_PPPOATM),y)
  CFLAGS += -DHAVE_PPPOATM
endif

ifeq ($(CONFIG_CHILLISPOT),y)
  CFLAGS += -DHAVE_CHILLILOCAL
  CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
endif

ifeq ($(CONFIG_DDLAN),y)
  CFLAGS += -DHAVE_DDLAN
endif
ifeq ($(CONFIG_CESAR),y)
  CFLAGS += -DHAVE_CESAR
endif


ifeq ($(CONFIG_34TELECOM),y)
  CFLAGS += -DHAVE_34TELECOM
endif

ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif
ifeq ($(CONFIG_BOESE),y)
CFLAGS += -DBOESE=1
endif
ifeq ($(CONFIG_WILLIAM),y)
CFLAGS += -DWILLIAM=1
endif
ifeq ($(CONFIG_ALPHA),y)
CFLAGS += -DHAVE_ALPHA
endif

ifeq ($(CONFIG_TELCOM),y)
CFLAGS += -DHAVE_TELCOM
endif

ifeq ($(CONFIG_GGEW),y)
  CFLAGS += -DHAVE_NEWMEDIA
  CFLAGS += -DHAVE_GGEW
endif


ifeq ($(CONFIG_SAGAR),y)
  CFLAGS += -DHAVE_SAGAR
endif

ifeq ($(CONFIG_OPENVPN),y)
  CFLAGS += -DHAVE_OPENVPN
endif

ifeq ($(CONFIG_THOM),y)
  CFLAGS += -DHAVE_THOM
endif

ifeq ($(CONFIG_CDEBUG),y)
  CFLAGS += -DCDEBUG
endif


ifeq ($(CONFIG_BOOT_WAIT_ON),y)
  CFLAGS += -DSET_BOOT_WAIT
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif
ifeq ($(CONFIG_UBNTM),y)
CFLAGS += -DHAVE_UBNTM
endif
ifeq ($(CONFIG_DAP3410),y)
CFLAGS += -DHAVE_DAP3410
endif
ifeq ($(CONFIG_DAP3310),y)
CFLAGS += -DHAVE_DAP3310
endif
ifeq ($(CONFIG_UBNTTI),y)
CFLAGS += -DHAVE_UBNTTI
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_WNR2000),y)
CFLAGS += -DHAVE_WNR2000
endif
ifeq ($(CONFIG_WNR2200),y)
CFLAGS += -DHAVE_WNR2200
endif
ifeq ($(CONFIG_HORNET),y)
CFLAGS += -DHAVE_HORNET
endif
ifeq ($(CONFIG_WASP),y)
CFLAGS += -DHAVE_WASP
endif
ifeq ($(CONFIG_WHRG300NV2),y)
CFLAGS += -DHAVE_WHRG300NV2
endif
ifeq ($(CONFIG_WLAEAG300N),y)
CFLAGS += -DHAVE_WLAEAG300N
endif
ifeq ($(CONFIG_WHRHPG300N),y)
CFLAGS += -DHAVE_WHRHPG300N
endif
ifeq ($(CONFIG_WHR300HP),y)
CFLAGS += -DHAVE_WHR300HP
endif
ifeq ($(CONFIG_WZRG450),y)
CFLAGS += -DHAVE_WZRG450
endif
ifeq ($(CONFIG_WZR450HP2),y)
CFLAGS += -DHAVE_WZR450HP2
endif
ifeq ($(CONFIG_ARCHERC7),y)
CFLAGS += -DHAVE_ARCHERC7
endif
ifeq ($(CONFIG_ARCHERC7V4),y)
CFLAGS += -DHAVE_ARCHERC7V4
endif
ifeq ($(CONFIG_MAC80211_MESH),y)
CFLAGS += -DHAVE_MAC80211_MESH
endif
ifeq ($(CONFIG_ARCHERC7V5),y)
CFLAGS += -DHAVE_ARCHERC7V5
endif
ifeq ($(CONFIG_ARCHERA7V5),y)
CFLAGS += -DHAVE_ARCHERA7V5
endif
ifeq ($(CONFIG_ARCHERC5),y)
CFLAGS += -DHAVE_ARCHERC5
endif
ifeq ($(CONFIG_WR1043V2),y)
CFLAGS += -DHAVE_WR1043V2
endif
ifeq ($(CONFIG_WR1043V3),y)
CFLAGS += -DHAVE_WR1043V3
endif
ifeq ($(CONFIG_MMS344),y)
CFLAGS += -DHAVE_MMS344
endif
ifeq ($(CONFIG_DIR632),y)
CFLAGS += -DHAVE_DIR632
endif
ifeq ($(CONFIG_WZRG300NH2),y)
CFLAGS += -DHAVE_WZRG300NH2
endif
ifeq ($(CONFIG_WZR300HP),y)
CFLAGS += -DHAVE_WZR300HP
endif
ifeq ($(CONFIG_TEW632BRP),y)
CFLAGS += -DHAVE_TEW632BRP
endif
ifeq ($(CONFIG_TEW652BRP),y)
CFLAGS += -DHAVE_TEW652BRP
endif
ifeq ($(CONFIG_DIR615C1),y)
CFLAGS += -DHAVE_DIR615C1
endif
ifeq ($(CONFIG_DIR615),y)
CFLAGS += -DHAVE_DIR615
endif
ifeq ($(CONFIG_DIR615H),y)
CFLAGS += -DHAVE_DIR615H
endif
ifeq ($(CONFIG_DIR601A1),y)
CFLAGS += -DHAVE_DIR601A1
endif
ifeq ($(CONFIG_DIR615E),y)
CFLAGS += -DHAVE_DIR615E
endif
ifeq ($(CONFIG_DIR615I),y)
CFLAGS += -DHAVE_DIR615I
endif
ifeq ($(CONFIG_DAP3320),y)
CFLAGS += -DHAVE_DAP3320
endif
ifeq ($(CONFIG_TEW824),y)
CFLAGS += -DHAVE_TEW824
endif
ifeq ($(CONFIG_DAP2230),y)
CFLAGS += -DHAVE_DAP2230
endif
ifeq ($(CONFIG_DAP2330),y)
CFLAGS += -DHAVE_DAP2330
endif
ifeq ($(CONFIG_DAP2660),y)
CFLAGS += -DHAVE_DAP2660
endif
ifeq ($(CONFIG_DAP2680),y)
CFLAGS += -DHAVE_DAP2680
endif
ifeq ($(CONFIG_WR841V8),y)
CFLAGS += -DHAVE_WR841V8
endif
ifeq ($(CONFIG_ARCHERC25),y)
CFLAGS += -DHAVE_ARCHERC25
endif
ifeq ($(CONFIG_WR810N),y)
CFLAGS += -DHAVE_WR810N
endif
ifeq ($(CONFIG_WA901V3),y)
CFLAGS += -DHAVE_WA901V3
endif
ifeq ($(CONFIG_WA850RE),y)
CFLAGS += -DHAVE_WA850RE
endif
ifeq ($(CONFIG_WA860RE),y)
CFLAGS += -DHAVE_WA860RE
endif
ifeq ($(CONFIG_WR841V9),y)
CFLAGS += -DHAVE_WR841V9
endif
ifeq ($(CONFIG_WR841V10),y)
CFLAGS += -DHAVE_WR841V10
endif
ifeq ($(CONFIG_WR841V11),y)
CFLAGS += -DHAVE_WR841V11
endif
ifeq ($(CONFIG_WR841V12),y)
CFLAGS += -DHAVE_WR841V12
endif
ifeq ($(CONFIG_WR842V2),y)
CFLAGS += -DHAVE_WR842V2
endif
ifeq ($(CONFIG_DIR615E1),y)
CFLAGS += -DHAVE_DIR615E1
endif
ifeq ($(CONFIG_TG2521),y)
CFLAGS += -DHAVE_TG2521
endif
ifeq ($(CONFIG_WR1043),y)
CFLAGS += -DHAVE_WR1043
endif
ifeq ($(CONFIG_WA901),y)
CFLAGS += -DHAVE_WA901
endif
ifeq ($(CONFIG_WR941),y)
CFLAGS += -DHAVE_WR941
endif
ifeq ($(CONFIG_WR941V6),y)
CFLAGS += -DHAVE_WR941V6
endif
ifeq ($(CONFIG_WA901V4),y)
CFLAGS += -DHAVE_WA901V4
endif
ifeq ($(CONFIG_WA901V5),y)
CFLAGS += -DHAVE_WA901V5
endif
ifeq ($(CONFIG_WR940V4),y)
CFLAGS += -DHAVE_WR940V4
endif
ifeq ($(CONFIG_WR940V6),y)
CFLAGS += -DHAVE_WR940V6
endif
ifeq ($(CONFIG_WR741),y)
CFLAGS += -DHAVE_WR741
endif
ifeq ($(CONFIG_WA7510),y)
CFLAGS += -DHAVE_WA7510
endif
ifeq ($(CONFIG_HABANERO),y)
CFLAGS += -DHAVE_HABANERO
endif
ifeq ($(CONFIG_WA701V2),y)
CFLAGS += -DHAVE_WA701V2
endif
ifeq ($(CONFIG_WR703),y)
CFLAGS += -DHAVE_WR703
endif
ifeq ($(CONFIG_WR710),y)
CFLAGS += -DHAVE_WR710
endif
ifeq ($(CONFIG_GL150),y)
CFLAGS += -DHAVE_GL150
endif
ifeq ($(CONFIG_WR71021),y)
CFLAGS += -DHAVE_WR71021
endif
ifeq ($(CONFIG_WR710V1),y)
CFLAGS += -DHAVE_WR710V1
endif
ifeq ($(CONFIG_MR3020),y)
CFLAGS += -DHAVE_MR3020
endif
ifeq ($(CONFIG_WR741V4),y)
CFLAGS += -DHAVE_WR741V4
endif
ifeq ($(CONFIG_WR740V4),y)
CFLAGS += -DHAVE_WR740V4
endif
ifeq ($(CONFIG_WR743),y)
CFLAGS += -DHAVE_WR743
endif
ifeq ($(CONFIG_WR743V2),y)
CFLAGS += -DHAVE_WR743V2
endif
ifeq ($(CONFIG_WR841v3),y)
CFLAGS += -DHAVE_WR841v3
endif
ifeq ($(CONFIG_WR840v1),y)
CFLAGS += -DHAVE_WR840v1
endif
ifeq ($(CONFIG_WR841v5),y)
CFLAGS += -DHAVE_WR841v5
endif
ifeq ($(CONFIG_WR841v7),y)
CFLAGS += -DHAVE_WR841v7
endif
ifeq ($(CONFIG_MR3420),y)
CFLAGS += -DHAVE_MR3420
endif
ifeq ($(CONFIG_MR3220),y)
CFLAGS += -DHAVE_MR3220
endif
ifeq ($(CONFIG_WR842),y)
CFLAGS += -DHAVE_WR842
endif
ifeq ($(CONFIG_IPETH),y)
CFLAGS += -DHAVE_IPETH
endif
ifeq ($(CONFIG_WA901v1),y)
CFLAGS += -DHAVE_WA901v1
endif
ifeq ($(CONFIG_WA801v1),y)
CFLAGS += -DHAVE_WA801v1
endif
ifeq ($(CONFIG_WR941v4),y)
CFLAGS += -DHAVE_WR941v4
endif
ifeq ($(CONFIG_WR740v1),y)
CFLAGS += -DHAVE_WR740v1
endif
ifeq ($(CONFIG_JJAP93),y)
CFLAGS += -DHAVE_JJAP93
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif
ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
endif
ifeq ($(CONFIG_MAKSAT_BLANK),y)
CFLAGS += -DHAVE_MAKSAT_BLANK
endif

ifeq ($(CONFIG_TMK),y)
CFLAGS += -DHAVE_TMK
endif

ifeq ($(CONFIG_BKM),y)
CFLAGS += -DHAVE_BKM
endif

ifeq ($(CONFIG_NOTRIAL),y)
CFLAGS += -DHAVE_NOTRIAL
endif
ifeq ($(CONFIG_CNC),y)
CFLAGS += -DHAVE_CNC
endif

ifeq ($(CONFIG_VILIM),y)
CFLAGS += -DHAVE_VILIM
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_BRANDING),y)
CFLAGS += -DHAVE_BRANDING
endif

ifeq ($(CONFIG_MASKMAC),y)
CFLAGS += -DFBNFW
endif

ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -DHAVE_HTTPS
endif

ifeq ($(CONFIG_MATRIXSSL),y)
CFLAGS += -DHAVE_HTTPS
CFLAGS += -I$(TOP)/matrixssl
CFLAGS += -DHAVE_MATRIXSSL


endif

ifeq ($(CONFIG_WTS),y)
CFLAGS += -DHAVE_WTS
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
endif

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
endif

ifeq ($(CONFIG_POWERNOC_WORT54G),y)
CFLAGS += -DHAVE_POWERNOC_WORT54G
endif

ifeq ($(CONFIG_POWERNOC_WOAP54G),y)
CFLAGS += -DHAVE_POWERNOC_WOAP54G
endif

ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif
ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif
ifeq ($(CONFIG_FONBETA),y)
CFLAGS += -DHAVE_FONBETA
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
endif

ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
endif

ifeq ($(CONFIG_GWMF54G2),y)
CFLAGS += -DHAVE_GWMF54G2
endif
ifeq ($(CONFIG_SVQOS),y)
CFLAGS += -DHAVE_SVQOS
endif
ifeq ($(CONFIG_NET_SCH_CODEL),y)
CFLAGS += -DHAVE_CODEL
endif
ifeq ($(CONFIG_NET_SCH_CODEL),m)
CFLAGS += -DHAVE_CODEL
endif
ifeq ($(CONFIG_NET_SCH_FQ_CODEL),y)
CFLAGS += -DHAVE_FQ_CODEL
endif
ifeq ($(CONFIG_NET_SCH_FQ_CODEL),m)
CFLAGS += -DHAVE_FQ_CODEL
endif
ifeq ($(CONFIG_NET_SCH_PIE),y)
CFLAGS += -DHAVE_PIE
endif
ifeq ($(CONFIG_NET_SCH_PIE),m)
CFLAGS += -DHAVE_PIE
endif
ifeq ($(CONFIG_CAKE),y)
CFLAGS += -DHAVE_CAKE
CFLAGS += -DHAVE_FQ_CODEL_FAST
endif
CFLAGS += -DHAVE_AQOS
ifeq ($(CONFIG_NVRAM_60K),y)
CFLAGS += -DHAVE_NVRAM_60K
endif
ifeq ($(CONFIG_NVRAM_64K),y)
CFLAGS += -DHAVE_NVRAM_64K
endif

ifeq ($(CONFIG_SUBQUARTER),y)
  CFLAGS += -DHAVE_SUBQUARTER
endif

include $(TOP)/rules/generic.mk
CFLAGS += $(DD_CFLAGS)

ifeq ($(CONFIG_MATRIXSSL),y)
#CFLAGS += -I$(TOP)/matrixssl
#LDFLAGS += -L$(TOP)/matrixssl/src -lmatrixssl
CFLAGS += -DHAVE_MATRIXSSL
endif

ifeq ($(CONFIG_SAMBA3),y)
  SHARETOOLSOBJS += libsharetools/samba3.o 
  CFLAGS += -I$(TOP)/jansson/src
  CFLAGS += -DHAVE_SAMBA3
  CFLAGS += -DHAVE_SAMBA_SERVER
  ADDLIB += libsharetools.so
endif
ifeq ($(CONFIG_FTP),y)
  SHARETOOLSOBJS += libsharetools/samba3.o 
  CFLAGS += -I$(TOP)/jansson/src
  ADDLIB += libsharetools.so
endif
ifeq ($(CONFIG_MINIDLNA),y)
  SHARETOOLSOBJS += libsharetools/dlna.o
  CFLAGS += -I$(TOP)/jansson/src 
  CFLAGS += -DHAVE_MINIDLNA
  ADDLIB += libsharetools.so
endif
ifeq ($(CONFIG_NFS),y)
  SHARETOOLSOBJS += libsharetools/nfs.o
  CFLAGS += -I$(TOP)/jansson/src 
  CFLAGS += -DHAVE_NFS
  ADDLIB += libsharetools.so
endif
ifeq ($(CONFIG_RSYNC),y)
  SHARETOOLSOBJS += libsharetools/rsync.o
  CFLAGS += -I$(TOP)/jansson/src
  CFLAGS += -DHAVE_RSYNC
  ADDLIB += libsharetools.so
endif




HFLAGS := $(CFLAGS)
CFLAGS += $(COPTS)

A_LIBS :=libshutils.a libutils.a libwireless.a
C_LIBS :=libshutils.so libutils.so libwireless.so libqos.so $(ADDLIB)
ifeq ($(CONFIG_USB),y)
ifneq ($(CONFIG_3G_ONLY),y)
C_LIBS +=libdisc.so
endif
endif
ifeq ($(CONFIG_RAID),y)
C_LIBS +=libdisc.so
endif
all: $(C_LIBS) $(A_LIBS)

install: all
	install -d $(INSTALLDIR)/libutils/usr/lib
	cp $(C_LIBS) $(INSTALLDIR)/libutils/usr/lib

clean:
#	rm -f countrycode/countrycode_$(ARCH).o
	rm -f libutils/*.o libqos/*.o libwireless/*.o libshutils/*.o libdisc/*.o libsharetools/*.o *.o *.so
	rm -f $(TOP)/register/*.o
	rm -f *.a
	rm -f *.o
	rm -f include/*.h~
	rm -f libqos/*.c~ libwireless/*.c~ libshutils/*.c~ libutils/*.c~ libsharetools/*.c~ libdisc/*.c~
	rm -f $(TOP)/private/telkom/*.o

libwireless/wl_ath9k.o:
	@echo "[CC] $@"
	@$(CC) -shared -c -Iinclude $(ATH9KCFLAGS) $(MIPS16_OPT) $(THUMB) -o libwireless/wl_ath9k.o libwireless/wl_ath9k.c

libwireless/mac80211info.o:
	@echo "[CC] $@"
	@$(CC) -shared -c -Iinclude $(ATH9KCFLAGS) $(MIPS16_OPT) $(THUMB) -o libwireless/mac80211info.o libwireless/mac80211info.c

libwireless/mac80211regulatory.o:
	@echo "[CC] $@"
	@$(CC) -shared -c -Iinclude $(ATH9KCFLAGS) $(MIPS16_OPT) $(THUMB) -o libwireless/mac80211regulatory.o libwireless/mac80211regulatory.c

libwireless/mac80211autochannel.o:
	@echo "[CC] $@"
	@$(CC) -shared -c -Iinclude $(ATH9KCFLAGS) $(MIPS16_OPT) $(THUMB) -o libwireless/mac80211autochannel.o libwireless/mac80211autochannel.c

%.o : %.c
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -DUSE_IOCTL_LINUX -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -c -o $@ $<

libsharetools.so: $(SHARETOOLSOBJS)
	@echo "[LD] $@"
	@$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^  -L../nvram -lnvram -L. -lshutils -L$(TOP)/jansson/src/.libs -ljansson -lm

libqos.so: libqos/qos.o
	@echo "[LD] $@"
	@$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^  -lcrypt -L../nvram -lnvram -L. -lshutils

libwireless.so: $(WOBJS) $(ATH9KOBJS)
	@echo "[LD] $@"
	echo "-L$(TOP)/nvram -lnvram -L$(TOP)/libutils -lshutils $(LDFLAGS) $(WLDFLAGS) $(ATH9KLDFLAGS) $(QTNFLAGS)" > libwireless_ld
	$(CC) -shared $(CFLAGS) $(LDFLAGS)  -o $@ $(WOBJS) $(NOBJS) $(ATH9KOBJS) -L../nvram -lnvram -L. -lshutils $(WLDFLAGS) $(ATH9KLDFLAGS) $(QTNFLAGS)

libutils.so: $(OBJS)
	@echo "[LD] $@"
	echo "-lcrypt -L$(TOP)/nvram -lnvram -L$(TOP)/libutils -lshutils $(LDFLAGS)" > libutils_ld
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^  -lcrypt -L../nvram -lnvram -L. -lshutils

libshutils.so: $(SHOBJS)
	@echo "[LD] $@"
	echo "-L$(TOP)/nvram -lnvram $(LDFLAGS)" > libshutils_ld
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^  -L../nvram -lnvram

libdisc.so: $(DISCOBJS)
	@echo "[LD] $@"
	@$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^

mchillispot.o: $(TOP)/private/telkom/mchillispot.c
	@echo "[CC] $@"
	@$(CC) -DNEED_PRINTF -I$(TOP)/jansson/src $(ATH9KCFLAGS) $(MIPS16_OPT) $(THUMB) -c -o $@ $^

libshutils.a: $(SHOBJS)
	$(AR) cr $@ $^

libutils.a: $(OBJS)
	$(AR) cr $@ $^

libwireless.a: $(WOBJS) $(ATH9KOBJS)
	$(AR) cr $@ $^

*.o: $(CY_DEPS)
