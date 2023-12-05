-include $(LINUXDIR)/.config
CONFIG_WR941=
CONFIG_SWCONFIG=
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
CONFIG_E325N=
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
CONFIG_WR841V10=
CONFIG_DAP2230=
CONFIG_DAP2662=
CONFIG_DAP2330=
CONFIG_DIR859=
CONFIG_DIR869=
CONFIG_GL150=
CONFIG_ERC=

#
# milli_httpd Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.6 2005/11/30 11:55:05 seg Exp $
#
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

COPTS += $(MIPS16_OPT) -D_GNU_SOURCE -fvisibility=hidden $(LTO) -Wno-unused-function -Wno-unused-variable

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)
COPTS +=$(call cc-option,-Werror=return-local-addr,) -g


# -Werror=maybe-uninitialized -Werror=int-conversion -Werror=array-bounds

BCFLAGS	= $(COPTS)  -DMEMLEAK_OVERRIDE -DSTATIC_BUILD



ifeq ($(CONFIG_80211AC),y)
BCFLAGS	+= -I. -I$(LINUXDIR)/brcm/$(ARCH)/include -Wall
BCFLAGS	+= -I. -I$(LINUXDIR)/brcm/$(ARCH)/common/include -Wall 
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
BCFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ -Imodules
else
BCFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ -Imodules
endif
else
BCFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ -Imodules
endif

ifeq ($(CONFIG_VFS),y)
BCFLAGS += -I$(TOP)/vfs -DHAVE_VFS
LDFLAGS += -L$(TOP)/vfs
endif

#CFLAGS += -Ilzma
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(TOP)/libutils -lcrypt -ldl -lutils -lwireless -lshutils
#LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(TOP)/libutils -lcrypt -ldl -lutils -lwireless -L$(TOP)/jansson/src/.libs/ -ljansson

BCFLAGS += -DLINUX -ffunction-sections -fdata-sections -Wl,--gc-sections -DSTATIC_BUILD

ifeq ($(CONFIG_DIST),"micro")
#BCFLAGS += -DNEED_NOPRINTF
endif
ifneq ($(CONFIG_DIST),"micro-special")
#BCFLAGS += -DNEED_NOPRINTF
endif


ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
LDFLAGS += -pthread -lpthread
endif
endif

CFLAGS = $(BCFLAGS) -I$(LINUXDIR)/brcm/shared/bcmwifi/include -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
CFLAGS += -DARCH_$(ARCHITECTURE)

ifeq ($(CONFIG_SPECIALEDITION),y)
CFLAGS += -DHAVE_SPECIALEDITION
ADDOBJ = special.o
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"
CFLAGS += -DDIST_OPT=\"$(CONFIG_DIST_OPT)\"

ifeq ($(CONFIG_NOAD),y)
CFLAGS += -DHAVE_NOAD
endif

ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
CFLAGS += -DHAVE_EXTHELP
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
CFLAGS += -DHAVE_EXTHELP
endif
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifeq ($(CONFIG_EXTHELP),y)
CFLAGS += -DHAVE_EXTHELP
endif
endif
endif

endif

ifeq ($(CONFIG_ROUTERSTYLE),y)
CFLAGS += -DHAVE_ROUTERSTYLE
endif

ifeq ($(CONFIG_LANGUAGE),y)
CFLAGS += -DHAVE_LANGUAGE
endif
ifeq ($(CONFIG_GLAUCO),y)
CFLAGS += -DHAVE_GLAUCO
endif
ifeq ($(CONFIG_NEPTUNE),y)
CFLAGS += -DHAVE_NEPTUNE
endif
ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
endif
ifeq ($(CONFIG_WIVIZ),y)
CFLAGS += -DHAVE_WIVIZ
endif
ifeq ($(CONFIG_SWCONFIG),y)
CFLAGS += -DHAVE_SWCONFIG
LDFLAGS += -L$(TOP)/swconfig -lswitch -L$(TOP)/libnl-tiny -lnl-tiny
endif
ifeq ($(CONFIG_WAVESAT),y)
CFLAGS += -DHAVE_WAVESAT
endif
ifeq ($(CONFIG_OVERCLOCKING),y)
CFLAGS += -DHAVE_OVERCLOCKING
endif
ifeq ($(CONFIG_RADLOCAL),y)
CFLAGS += -DHAVE_RADLOCAL
endif
ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS += -DHAVE_WZRHPAG300NH
endif
ifneq ($(CONFIG_MADWIFI),y)
ifeq ($(CONFIG_80211AC),y)
CFLAGS += -DHAVE_80211AC
endif
endif
ifeq ($(CONFIG_BRCMFMAC),y)
CFLAGS += -DHAVE_BRCMFMAC
endif

ifeq ($(CONFIG_NOAC),y)
CFLAGS += -DHAVE_NOAC
endif
ifeq ($(CONFIG_SPEEDCHECKER),y)
CFLAGS += -DHAVE_SPEEDCHECKER
endif
ifeq ($(CONFIG_SPEEDTEST_CLI),y)
VISOBJS += speedtest.o
CFLAGS += -DHAVE_SPEEDTEST_CLI
endif
ifeq ($(CONFIG_WZRG450),y)
CFLAGS += -DHAVE_WZRG450
endif
ifeq ($(CONFIG_WZR450HP2),y)
CFLAGS += -DHAVE_WZR450HP2
endif
ifeq ($(CONFIG_WZRG300NH2),y)
CFLAGS += -DHAVE_WZRG300NH2
endif
ifeq ($(CONFIG_NO_STYLUS),y)
CFLAGS += -DHAVE_NO_STYLUS
endif
ifeq ($(CONFIG_WPA_SUPPLICANT),y)
CFLAGS += -DHAVE_WPA_SUPPLICANT
endif
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
CFLAGS += -DHAVE_WPA_SUPPLICANT
endif
ifeq ($(CONFIG_NOTRIAL),y)
CFLAGS += -DHAVE_NOTRIAL
endif
ifeq ($(CONFIG_ACK),y)
CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_V25),y)
CFLAGS += -DHAVE_V25
endif
ifeq ($(CONFIG_DIR860),y)
CFLAGS += -DHAVE_DIR860
endif
ifeq ($(CONFIG_DIR859),y)
CFLAGS += -DHAVE_DIR859
endif
ifeq ($(CONFIG_DIR869),y)
CFLAGS += -DHAVE_DIR869
endif
ifeq ($(CONFIG_IPVS),y)
CFLAGS += -DHAVE_IPVS
endif
ifeq ($(CONFIG_DIR862),y)
CFLAGS += -DHAVE_DIR862
endif
ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DHAVE_BCMMODERN
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS += -DHAVE_SYSLOG -DHAVE_STATUS_SYSLOG
endif
ifeq ($(CONFIG_OCTEON),y)
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_OCTEON
endif
ifeq ($(CONFIG_EROUTER),y)
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_EROUTER -DHAVE_UPX86
endif
ifeq ($(CONFIG_MULTICAST),y)
CFLAGS += -DHAVE_MULTICAST
endif
ifeq ($(CONFIG_NP25G),y)
CFLAGS += -DHAVE_NP25G
endif
ifeq ($(CONFIG_UNIWIP),y)
CFLAGS += -DHAVE_UNIWIP
endif
ifeq ($(CONFIG_MMS344),y)
CFLAGS += -DHAVE_MMS344
endif
ifeq ($(CONFIG_CPE890),y)
CFLAGS += -DHAVE_CPE890
endif
ifeq ($(CONFIG_SR3200),y)
CFLAGS += -DHAVE_SR3200
endif
ifeq ($(CONFIG_WILLY),y)
CFLAGS += -DHAVE_WILLY
endif
ifeq ($(CONFIG_XD3200),y)
CFLAGS += -DHAVE_XD3200
endif
ifeq ($(CONFIG_AP120C),y)
CFLAGS += -DHAVE_AP120C
endif
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
endif
ifeq ($(CONFIG_MAC80211_MESH),y)
CFLAGS += -DHAVE_MAC80211_MESH
endif
ifeq ($(CONFIG_HOTSPOT),y)
CFLAGS += -DHAVE_HOTSPOT
endif
ifeq ($(CONFIG_HOTPLUG2),y)
CFLAGS += -DHAVE_HOTPLUG2
endif
ifeq ($(CONFIG_WRT54G2),y)
CFLAGS += -DHAVE_WRT54G2
endif
ifeq ($(CONFIG_VF803),y)
  CFLAGS += -DHAVE_VF803
endif
ifeq ($(CONFIG_MSTP),y)
  CFLAGS += -DHAVE_MSTP
endif
ifeq ($(CONFIG_TOR),y)
  CFLAGS += -DHAVE_TOR
endif
ifeq ($(CONFIG_DNSCRYPT),y)
CFLAGS += -DHAVE_DNSCRYPT
endif
ifeq ($(CONFIG_SMARTDNS),y)
CFLAGS += -DHAVE_SMARTDNS
endif
ifeq ($(CONFIG_GWMF54G2),y)
CFLAGS += -DHAVE_GWMF54G2
endif
ifeq ($(CONFIG_USBIP),y)
CFLAGS += -DHAVE_USBIP
endif
ifeq ($(CONFIG_ESR6650),y)
  CFLAGS += -DHAVE_ESR6650
endif
ifeq ($(CONFIG_SUBQUARTER),y)
  CFLAGS += -DHAVE_SUBQUARTER
endif
ifeq ($(CONFIG_SANSFIL),y)
  CFLAGS += -DHAVE_SANSFIL
endif
ifeq ($(CONFIG_MVEBU),y)
CFLAGS += -DHAVE_MVEBU
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_IPQ806X),y)
CFLAGS += -DHAVE_IPQ806X
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_ATH10K),y)
CFLAGS += -DHAVE_CPUTEMP
endif
include $(TOP)/rules/generic.mk
CFLAGS += $(DD_CFLAGS)

ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DDEFAULT_COUNTRY_CODE=\"$(CONFIG_DEFAULT_COUNTRYCODE)\"
VISOBJS += bfwireless.o onlineupdateejs.o
WEBSOBJS += onlineupdatewebs.o
OBJS += onlineupdatecgi.o
ifeq ($(CONFIG_IAS),y)
VISOBJS += intatstart.o
WEBSOBJS += intatstartwebs.o
endif
ifeq ($(CONFIG_GUESTPORT),y)
CFLAGS += -DHAVE_GUESTPORT
endif
endif

ifeq ($(CONFIG_FREECWMP),y)
CFLAGS += -DHAVE_FREECWMP
VISOBJS += freecwmp.o
endif

ifeq ($(CONFIG_UNFY),y)
CLFAGS += -DHAVE_UNFY
endif

ifeq ($(CONFIG_IDEXX),y)
CFLAGS += -DHAVE_IDEXX
ifeq ($(CONFIG_IDEXX_WORLD),y)
CFLAGS += -DHAVE_IDEXX_WORLD
endif
endif

ifeq ($(CONFIG_IDEXX_SIGNATUR),y)
CFLAGS += -DHAVE_IDEXX_SIGNATUR
endif

ifeq ($(CONFIG_FREECWMP),y)
CFLAGS += -DHAVE_FREECWMP
VISOBJS += freecwmp.o
endif

ifeq ($(CONFIG_UNFY),y)
CLFAGS += -DHAVE_UNFY
endif


ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500 -DHAVE_UPRB500
else
ifeq ($(CONFIG_WDR4900),y)
else


ifneq ($(CONFIG_X86),y)
ifneq ($(CONFIG_RB600),y)
ifneq ($(CONFIG_EROUTER),y)
ifeq ($(CONFIG_BUFFALO),y)
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
OBJS += upgrade_buffalo.o
else
endif
else
endif
CFLAGS += -DHAVE_LOUD
endif
endif
endif
endif
endif
ifeq ($(CONFIG_DDLINK),y)
  CFLAGS += -DHAVE_DDLINK
endif
ifeq ($(CONFIG_WPS),y)
  CFLAGS += -DHAVE_WPS
endif
ifeq ($(CONFIG_AOSS),y)
  CFLAGS += -DHAVE_AOSS
endif
ifeq ($(CONFIG_IAS),y)
  CFLAGS += -DHAVE_IAS
ifeq ($(CONFIG_IAS_NOPWD),y)
  CFLAGS += -DCONFIG_IAS_NOPLNPWD
endif
endif
ifeq ($(CONFIG_RELAYD),y)
  CFLAGS += -DHAVE_RELAYD
endif
ifeq ($(CONFIG_LAGUNA),y)
  CFLAGS += -DHAVE_EXT_IPROUTE
  CFLAGS += -DHAVE_LAGUNA
  CFLAGS += -DHAVE_CPUTEMP
  CFLAGS += -DHAVE_VOLT
endif
ifeq ($(CONFIG_VENTANA),y)
  CFLAGS += -DHAVE_EXT_IPROUTE
  CFLAGS += -DHAVE_VENTANA
  CFLAGS += -DHAVE_CPUTEMP
  CFLAGS += -DHAVE_VOLT
endif
ifeq ($(CONFIG_NEWPORT),y)
  CFLAGS += -DHAVE_EXT_IPROUTE
  CFLAGS += -DHAVE_VENTANA -DHAVE_UPX86
  CFLAGS += -DHAVE_NEWPORT
  CFLAGS += -DHAVE_CPUTEMP
  CFLAGS += -DHAVE_VOLT
endif
ifeq ($(CONFIG_AP83),y)
  CFLAGS += -DHAVE_AP83
endif
ifeq ($(CONFIG_THOM),y)
  CFLAGS += -DHAVE_THOM
endif
ifeq ($(CONFIG_WRT160NL),y)
  CFLAGS += -DHAVE_WRT160NL
endif
ifeq ($(CONFIG_BONDING),y)
  CFLAGS += -DHAVE_BONDING
endif
ifeq ($(CONFIG_MAC80211_COMPRESS),y)
  CFLAGS += -DHAVE_MAC80211_COMPRESS
endif
ifeq ($(CONFIG_IPETH),y)
  CFLAGS += -DHAVE_IPETH
endif
ifeq ($(CONFIG_RTG32),y)
  CFLAGS += -DHAVE_RTG32
endif

ifeq ($(CONFIG_XSCALE),y)
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_FREERADIUS),y)
CFLAGS += -DHAVE_FREERADIUS
VISOBJS += freeradius.o
WEBSOBJS += websfreeradius.o
endif
ifeq ($(CONFIG_POKER),y)
CFLAGS += -DHAVE_POKER
VISOBJS += poker.o
WEBSOBJS += webspoker.o
endif
ifeq ($(CONFIG_PICO5),y)
CFLAGS += -DHAVE_PICO5
endif
ifeq ($(CONFIG_PICO2),y)
CFLAGS += -DHAVE_PICO2
endif
ifeq ($(CONFIG_OPENDPI),y)
CFLAGS += -DHAVE_OPENDPI
endif
ifeq ($(CONFIG_BATMANADV),y)
CFLAGS += -DHAVE_BATMANADV
endif
ifeq ($(CONFIG_NEXTMEDIA),y)
CFLAGS += -DHAVE_NEXTMEDIA
endif
ifeq ($(CONFIG_RAIEXTRA),y)
CFLAGS += -DHAVE_RAIEXTRA
endif
ifeq ($(CONFIG_PICO2HP),y)
CFLAGS += -DHAVE_PICO2HP
endif
ifeq ($(CONFIG_CAMBRIA),y)
  CFLAGS += -DHAVE_CAMBRIA
endif
ifeq ($(CONFIG_PWC),y)
CFLAGS += -DHAVE_PWC
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
VISOBJS += raid.o
ifeq ($(CONFIG_RAID),y)
CFLAGS += -DHAVE_RAID
WEBSLDFLAGS += -ldisc
WEBSOBJS += websraid.o
endif
ifeq ($(CONFIG_ZFS),y)
CFLAGS += -DHAVE_ZFS
endif
ifeq ($(CONFIG_ERC),y)
CFLAGS += -DHAVE_ERC
endif
ifeq ($(CONFIG_RUT500),y)
CFLAGS += -DHAVE_RUT500
endif
ifeq ($(CONFIG_3G),y)
CFLAGS += -DHAVE_3G
endif
ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
endif
ifeq ($(CONFIG_MAKSAT_BLANK),y)
CFLAGS += -DHAVE_MAKSAT_BLANK
endif
ifeq ($(CONFIG_NOCOUNTRYSEL),y)
CFLAGS += -DHAVE_NOCOUNTRYSEL
endif
ifdef CONFIG_DEFREGDOMAIN
CFLAGS += -DHAVE_DEFREGDOMAIN=$(CONFIG_DEFREGDOMAIN)
endif
ifeq ($(CONFIG_GPSI),y)
CFLAGS += -DHAVE_GPSI
endif
ifeq ($(CONFIG_TRIMAX),y)
CFLAGS += -DHAVE_TRIMAX
endif
ifeq ($(CONFIG_IPV6),y)
CFLAGS += -DHAVE_IPV6
endif
ifeq ($(CONFIG_DNSSEC),y)
CFLAGS += -DHAVE_DNSSEC
endif
ifeq ($(CONFIG_ESPOD),y)
CFLAGS += -DHAVE_ESPOD
endif
ifeq ($(CONFIG_WIKINGS),y)
CFLAGS += -DHAVE_WIKINGS
endif
ifeq ($(CONFIG_SUB3),y)
CFLAGS += -DHAVE_SUB3
endif
ifeq ($(CONFIG_SUB6),y)
CFLAGS += -DHAVE_SUB6
endif
ifeq ($(CONFIG_SUB9),y)
CFLAGS += -DHAVE_SUB9
endif
ifeq ($(CONFIG_SUB12),y)
CFLAGS += -DHAVE_SUB12
endif
ifeq ($(CONFIG_CARLSONWIRELESS),y)
CFLAGS += -DHAVE_CARLSONWIRELESS
endif
ifeq ($(CONFIG_IMMERSIVE),y)
CFLAGS += -DHAVE_IMMERSIVE
endif
ifeq ($(CONFIG_IPR),y)
CFLAGS += -DHAVE_IPR
endif
ifeq ($(CONFIG_ENEO),y)
CFLAGS += -DHAVE_ENEO
endif
ifeq ($(CONFIG_ONNET),y)
CFLAGS += -DHAVE_ONNET
endif
ifeq ($(CONFIG_ONNET_BLANK),y)
CFLAGS += -DHAVE_ONNET_BLANK
endif
ifeq ($(CONFIG_RAYTRONIK),y)
CFLAGS += -DHAVE_RAYTRONIK
endif
ifeq ($(CONFIG_VILIM),y)
CFLAGS += -DHAVE_VILIM
endif
ifeq ($(CONFIG_ANTAIRA),y)
CFLAGS += -DHAVE_ANTAIRA
#WEBSOBJS += wpa_passphrase.o
ifeq ($(CONFIG_ANTAIRA_AGENT),y)
CFLAGS += -DHAVE_ANTAIRA_AGENT
endif
endif
ifeq ($(CONFIG_FMS2111),y)
CFLAGS += -DHAVE_FMS2111
endif
ifeq ($(CONFIG_ANTAIRA_MINI),y)
CFLAGS += -DHAVE_ANTAIRA_MINI
endif
ifeq ($(CONFIG_TMK),y)
CFLAGS += -DHAVE_TMK
endif
ifeq ($(CONFIG_BKM),y)
CFLAGS += -DHAVE_BKM
endif
ifeq ($(CONFIG_KORENRON),y)
CFLAGS += -DHAVE_KORENRON
endif
ifeq ($(CONFIG_TESTEM),y)
CFLAGS += -DHAVE_TESTEM
endif
ifeq ($(CONFIG_HDWIFI),y)
CFLAGS += -DHAVE_HDWIFI
endif
ifeq ($(CONFIG_HOBBIT),y)
CFLAGS += -DHAVE_HOBBIT
endif
#ifeq ($(CONFIG_MRP),y)
#CFLAGS += -DHAVE_MRP
#VISOBJS += mrp.o
#WEBSOBJS += websmrp.o
#endif
ifeq ($(CONFIG_STATUS_GPIO),y)
CFLAGS += -DHAVE_STATUS_GPIO
endif
ifeq ($(CONFIG_SOLO51),y)
CFLAGS += -DHAVE_SOLO51
endif
ifeq ($(CONFIG_RSTATS),y)
CFLAGS += -DHAVE_RSTATS
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_PORTSETUP
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif

ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_EXT_IPROUTE
CFLAGS += -DHAVE_X86 -DHAVE_UPX86
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -DHAVE_RB600

ifneq ($(CONFIG_WDR4900),y)
CFLAGS += -DHAVE_UPX86
endif
endif

ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_WDR4900),y)
CFLAGS += -DHAVE_WDR4900
endif
ifeq ($(CONFIG_CPUTEMP),y)
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
CFLAGS += -DHAVE_RADIUSPLUGIN
endif
ifeq ($(CONFIG_FONERA),y)
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_ALPHA),y)
CFLAGS += -DHAVE_ALPHA
endif
ifeq ($(CONFIG_WIL6210),y)
  CFLAGS += -DHAVE_WIL6210
endif
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -DHAVE_ALPINE
endif
ifeq ($(CONFIG_EOC2610),y)
CFLAGS += -DHAVE_EOC2610
endif
ifeq ($(CONFIG_EOC1650),y)
CFLAGS += -DHAVE_EOC1650
endif
ifeq ($(CONFIG_ALLNETWRT),y)
CFLAGS += -DHAVE_ALLNETWRT
endif
ifeq ($(CONFIG_FONERA2200),y)
CFLAGS += -DHAVE_FONERA2200
endif
ifeq ($(CONFIG_DIR400),y)
CFLAGS += -DHAVE_DIR400
endif
ifeq ($(CONFIG_DIR825),y)
  CFLAGS += -DHAVE_DIR825
endif
ifeq ($(CONFIG_XR4SPECIAL),y)
CFLAGS += -DHAVE_XR4SPECIAL
endif
ifeq ($(CONFIG_DIR300),y)
CFLAGS += -DHAVE_DIR300
endif
ifeq ($(CONFIG_USR5453),y)
CFLAGS += -DHAVE_USR5453
endif
ifeq ($(CONFIG_MR3202A),y)
CFLAGS += -DHAVE_MR3202A
endif
ifeq ($(CONFIG_WRK54G),y)
CFLAGS += -DHAVE_WRK54G
endif
ifeq ($(CONFIG_MERAKI),y)
CFLAGS += -DHAVE_MERAKI
endif
ifeq ($(CONFIG_LS2),y)
CFLAGS += -DHAVE_LS2
endif
ifeq ($(CONFIG_WP543),y)
  CFLAGS += -DHAVE_WP543
endif
ifeq ($(CONFIG_WP546),y)
  CFLAGS += -DHAVE_WP546
endif
ifeq ($(CONFIG_NS2),y)
CFLAGS += -DHAVE_NS2
endif
ifeq ($(CONFIG_LC2),y)
CFLAGS += -DHAVE_LC2
endif
ifeq ($(CONFIG_LC5),y)
CFLAGS += -DHAVE_LC5
endif
ifeq ($(CONFIG_NS5),y)
CFLAGS += -DHAVE_NS5
endif
ifeq ($(CONFIG_EOC5610),y)
CFLAGS += -DHAVE_EOC5610
endif
ifeq ($(CONFIG_LS5),y)
CFLAGS += -DHAVE_LS5
endif
ifeq ($(CONFIG_WHRAG108),y)
CFLAGS += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS += -DHAVE_TW6600
endif
ifeq ($(CONFIG_CA8),y)
CFLAGS += -DHAVE_CA8
endif
ifeq ($(CONFIG_CA8PRO),y)
CFLAGS += -DHAVE_CA8PRO
endif
ifeq ($(CONFIG_NOP8670),y)
CFLAGS += -DHAVE_NOP8670
CFLAGS += -DHAVE_GATEWORX
else
ifeq ($(CONFIG_TONZE),y)
CFLAGS += -DHAVE_TONZE
endif
ifeq ($(CONFIG_WG302),y)
CFLAGS += -DHAVE_WG302
endif
ifeq ($(CONFIG_DSL_CPE_CONTROL),y)
CFLAGS += -DHAVE_DSL_CPE_CONTROL
endif

ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_GATEWORX
ifneq ($(CONFIG_WRT300NV2),y)
ifneq ($(CONFIG_WG302),y)
CFLAGS += -DHAVE_CPUTEMP
CFLAGS += -DHAVE_VOLT
endif
endif
endif
endif

ifeq ($(REGDOMAIN_OVERRIDE),y)
CFLAGS += -DREGDOMAIN_OVERRIDE
endif

ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_VLANTAGGING
endif
#ifeq ($(CONFIG_MDHCP),y)
CFLAGS += -DHAVE_MDHCP
#endif

ifeq ($(CONFIG_DDLAN),y)
CFLAGS += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_CORENET),y)
CFLAGS += -DHAVE_CORENET
endif
ifeq ($(CONFIG_XIOCOM),y)
CFLAGS += -DHAVE_XIOCOM
endif

ifeq ($(CONFIG_GGEW),y)
CFLAGS += -DHAVE_GGEW
endif
ifeq ($(CONFIG_TELCOM),y)
CFLAGS += -DHAVE_TELCOM
endif
ifeq ($(CONFIG_MYNETWAY),y)
  CFLAGS += -DHAVE_MYNETWAY
endif


ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
CFLAGS += -DHAVE_PPPOEDUAL
endif
ifeq ($(CONFIG_PPPOATM),y)
CFLAGS += -DHAVE_PPPOATM
endif
ifeq ($(CONFIG_MODEMBRIDGE),y)
CFLAGS += -DHAVE_MODEMBRIDGE
endif
ifeq ($(CONFIG_WMBR_G300NH),y)
CFLAGS += -DHAVE_WMBR_G300NH
endif

ifeq ($(CONFIG_CTORRENT),y)
CFLAGS += -DHAVE_CTORRENT
endif

ifeq ($(CONFIG_PWC),y)
CFLAGS += -DHAVE_PWC
endif

ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
endif

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
endif
ifeq ($(CONFIG_QUAGGA),y)
CFLAGS += -DHAVE_QUAGGA
endif
ifeq ($(CONFIG_FRR),y)
CFLAGS += -DHAVE_QUAGGA
endif
ifeq ($(CONFIG_PPTP),y)
CFLAGS += -DHAVE_PPTP
endif
ifeq ($(CONFIG_PPTPD),y)
CFLAGS += -DHAVE_PPTPD
VISOBJS += pptp.o
endif
ifeq ($(CONFIG_L2TP),y)
CFLAGS += -DHAVE_L2TP
endif
ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS += -DHAVE_HEARTBEAT
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
CFLAGS += -DHAVE_EOP_TUNNEL
VISOBJS += eoptunnel.o
endif
ifeq ($(CONFIG_MDNS),y)
CFLAGS += -DHAVE_MDNS
VISOBJS += mdnsif.o
endif
ifeq ($(CONFIG_WIREGUARD),y)
CFLAGS += -DHAVE_WIREGUARD
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
CFLAGS += -DHAVE_CHILLILOCAL
endif
ifeq ($(CONFIG_DLM101),y)
  CFLAGS += -DHAVE_DLM101
endif

ifeq ($(CONFIG_QTN),y)
CFLAGS += -DHAVE_QTN  -DRPCOVERRIDE
endif

ifeq ($(CONFIG_SMBD),y)
CFLAGS += -DHAVE_SMBD
endif

ifeq ($(CONFIG_FTP),y)
WEBSLDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
WEBSLDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson  -lsharetools
CFLAGS += -DHAVE_NAS_SERVER
endif

ifeq ($(CONFIG_SAMBA3),y)
WEBSLDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs 
WEBSLDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson -lsharetools
CFLAGS += -DHAVE_NAS_SERVER
CFLAGS += -DHAVE_SAMBA_SERVER
VISOBJS += samba3.o
endif

ifeq ($(CONFIG_MINIDLNA),y)
WEBSLDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
WEBSLDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson -lsharetools
CFLAGS += -DHAVE_MINIDLNA
VISOBJS += minidlna.o
endif

ifeq ($(CONFIG_PRIVOXY),y)
CFLAGS += -DHAVE_PRIVOXY
endif

ifeq ($(CONFIG_NFS),y)
CFLAGS += -DHAVE_NFS
VISOBJS += nfs.o
endif
#ifeq ($(CONFIG_SYSCTL_EDIT),y)
CFLAGS += -DHAVE_SYSCTL_EDIT
VISOBJS += sysctl.o
#endif

ifeq ($(CONFIG_RSYNC),y)
CFLAGS += -DHAVE_RSYNC
VISOBJS += rsync.o
endif

ifeq ($(CONFIG_WEBSERVER),y)
CFLAGS += -DHAVE_WEBSERVER
endif

#ifeq ($(CONFIG_SPUTNIK_APD),y)
#CFLAGS += -DHAVE_SPUTNIK_APD
#VISOBJS += sputnik.o
#endif
ifeq ($(CONFIG_USB),y)
CFLAGS += -DHAVE_USB
endif
ifeq ($(CONFIG_USB_ADVANCED),y)
CFLAGS += -DHAVE_USB_ADV
endif
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
CFLAGS += -DHAVE_SSHD
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER -DNOMAIN
#OBJS += crc.o register.c
REGOBJS = $(TOP)/register/register.c
REGOBJS2 = $(TOP)/register/register_vis.o
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
#REGOBJS2 = $(TOP)/register/register_check.c
endif
endif
endif
ifeq ($(CONFIG_SUPERCHANNEL),y)
CFLAGS += -DHAVE_SUPERCHANNEL
#OBJS += crc.o register.c
REGOBJS = $(TOP)/register/register.c
REGOBJS2 = $(TOP)/register/register_vis.o
endif

ifeq ($(CONFIG_MASKMAC),y)
CFLAGS += -DMASKMAC
CFLAGS += -DFBNFW
endif


ifeq ($(DDM_SUPPORT),1)
OBJS  += ddm.o
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_WRT300NV2),y)
CFLAGS += -DHAVE_WRT300NV2
endif
ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif
ifeq ($(CONFIG_NORTHSTAR),y)
CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_MUSL),y)
CFLAGS += -DHAVE_MUSL
endif

ifeq ($(CONFIG_OPENVPN),y)
ifeq ($(CONFIG_MUSL),y)
VISOBJS += speedtest.o
CFLAGS += -DHAVE_SPEEDTEST_CLI
endif
CFLAGS += -DHAVE_OPENVPN
endif


ifeq ($(CONFIG_BOESE),y)
CFLAGS += -DBOESE=1
endif
ifeq ($(CONFIG_WILLIAM),y)
CFLAGS += -DWILLIAM=1
endif
ifeq ($(ARCHITECTURE),broadcom)
ifneq ($(CONFIG_MADWIFI),y)
VISOBJS += wireless_broadcom.o
CFLAGS += -DHAVE_BRCM
endif
endif
ifeq ($(ARCHITECTURE),northstar)
ifneq ($(CONFIG_MADWIFI),y)
VISOBJS += wireless_broadcom.o
CFLAGS += -DHAVE_BRCM
endif
endif

ifeq ($(ARCHITECTURE),broadcom)
CFLAGS += -DHAVE_BRCM
endif
ifeq ($(ARCHITECTURE),northstar)
CFLAGS += -DHAVE_BRCM
endif
ifeq ($(ARCHITECTURE),broadcom)
ifneq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DLEGACY_SWITCH
endif
endif
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -DLEGACY_SWITCH
endif
ifeq ($(CONFIG_ATH9K),y)
LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
ATH9KVISOBJS += wireless_madwifiath9k.o
ATH9KVISFILES += visuals/wireless_madwifiath9k.c
CFLAGS += -I$(TOP)/libnl-tiny/include
endif


ifeq ($(CONFIG_MADWIFI),y)
ifneq ($(ARCHITECTURE),broadcom)
CFLAGS += -DHAVE_RELAYD
endif
CFLAGS += -DHAVE_MADWIFI -DHAVE_ACK -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h
VISOBJS += wireless_madwifi.o

#OBJS += ../wireless-tools/libiw.so.29
endif

ifeq ($(CONFIG_CDEBUG),y)
CFLAGS += -DCDEBUG
endif

CFLAGS += -DHAVE_AQOS

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

ifeq ($(CONFIG_WTS),y)
CFLAGS += -DHAVE_WTS
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif


ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
endif
ifeq ($(CONFIG_BUFFALO_SA),y)
CFLAGS += -DHAVE_BUFFALO_SA
endif
ifeq ($(CONFIG_SPOTPASS),y)
CFLAGS += -DHAVE_SPOTPASS
endif

ifeq ($(CONFIG_EASY_WIRELESS_CONFIG),y)
CFLAGS += -DHAVE_EASY_WIRELESS_CONFIG
endif
ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_EASY_WIRELESS_CONFIG
endif
ifeq ($(CONFIG_MADWIFI_ATH9K),y)
CFLAGS += -DHAVE_EASY_WIRELESS_CONFIG
endif

ifeq ($(CONFIG_IFL),y)
CFLAGS += -DHAVE_IFL
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_NOTRANS),y)
CFLAGS += -DHAVE_NOTRANS
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif
ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -DHAVE_REAL_OPENSSL
endif

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
endif
ifeq ($(CONFIG_HABANERO),y)
CFLAGS += -DHAVE_HABANERO
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

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DCONFIG_FREEBIRD
endif

ifeq ($(CONFIG_BRANDING),y)
CFLAGS += -DHAVE_BRANDING
endif

ifeq ($(ARP_TABLE_SUPPORT),1)
OBJS  += arp.o
endif
ifeq ($(CONFIG_UBNTM),y)
CFLAGS += -DHAVE_UBNTM
endif

ifeq ($(EMI_TEST),1)
OBJS  += emi_test.o
endif
ifeq ($(DEBUG_WEB),1)
CFLAGS += -DMY_DEBUG
endif
ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
endif
ifeq ($(CONFIG_PB42),y)
CFLAGS += -DHAVE_PB42
endif
ifeq ($(CONFIG_WPA3),y)
ifeq ($(HOSTAPDVERSION),)
CFLAGS += -DHAVE_WPA3 -DHAVE_80211R -DHAVE_80211W -DHAVE_USTEER
VISOBJS += roaming.o
endif
endif
ifeq ($(CONFIG_USTEER),y)
CFLAGS += -DHAVE_USTEER
VISOBJS += roaming.o
endif
ifeq ($(CONFIG_RS),y)
CFLAGS += -DHAVE_RS
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
ifeq ($(CONFIG_UNBOUND),y)
  CFLAGS += -DHAVE_UNBOUND
endif
ifeq ($(CONFIG_LSX),y)
CFLAGS += -DHAVE_LSX
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_WHR300HP2),y)
CFLAGS += -DHAVE_WHR300HP2
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_WZR300HP),y)
CFLAGS += -DHAVE_WZR300HP
endif
ifeq ($(CONFIG_WZR600DHP),y)
CFLAGS += -DHAVE_WZR600DHP
endif
ifeq ($(CONFIG_MT7620),y)
CFLAGS += -DHAVE_MT7620
endif
ifeq ($(CONFIG_MT76),y)
CFLAGS += -DHAVE_MT76
ifeq ($(CONFIG_MT7615),y)
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_MT7915),y)
CFLAGS += -DHAVE_CPUTEMP
endif
endif
ifeq ($(CONFIG_HORNET),y)
CFLAGS += -DHAVE_HORNET
endif
ifeq ($(CONFIG_DW02_412H),y)
CFLAGS += -DHAVE_DW02_412H
endif
ifeq ($(CONFIG_WASP),y)
CFLAGS += -DHAVE_WASP
endif
ifeq ($(CONFIG_WR650AC),y)
CFLAGS += -DHAVE_WR650AC
endif
ifeq ($(CONFIG_E355AC),y)
CFLAGS += -DHAVE_E355AC
endif
ifeq ($(CONFIG_E325N),y)
CFLAGS += -DHAVE_E325N
endif
ifeq ($(CONFIG_E380AC),y)
CFLAGS += -DHAVE_E380AC
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
ifeq ($(CONFIG_WP54G),y)
CFLAGS += -DHAVE_WP54G
endif
ifeq ($(CONFIG_ADM5120),y)
CFLAGS += -DHAVE_ADM5120
endif
ifeq ($(CONFIG_JJAP93),y)
CFLAGS += -DHAVE_JJAP93
endif
ifeq ($(CONFIG_DAP2230),y)
CFLAGS += -DHAVE_DAP2230
endif
ifeq ($(CONFIG_DAP3320),y)
CFLAGS += -DHAVE_DAP3320
endif
ifeq ($(CONFIG_DAP2330),y)
CFLAGS += -DHAVE_DAP2330
endif
ifeq ($(CONFIG_DAP2660),y)
CFLAGS += -DHAVE_DAP2660
endif
ifeq ($(CONFIG_DAP3662),y)
  CFLAGS += -DHAVE_DAP3662
endif


ifeq ($(CONFIG_TIEXTRA1),y)
WEBSCFLAGS += -DHAVE_TIEXTRA1 -Wl,-rpath,$(TOP)/jansson/src/.libs
WEBSLDFLAGS +=   -Wl,-rpath,$(TOP)/jansson/src/.libs
VISOBJS  += $(TOP)/private/telkom/mchillispotejs.o
WEBSOBJS  += $(TOP)/private/telkom/mchillispotvalidate.o
endif
ifeq ($(CONFIG_TIEXTRA2),y)
WEBSCFLAGS += -DHAVE_TIEXTRA2 -Wl,-rpath,$(TOP)/jansson/src/.libs
WEBSLDFLAGS +=   -Wl,-rpath,$(TOP)/jansson/src/.libs
VISOBJS  += $(TOP)/private/telkom/mwifidogejs.o
WEBSOBJS  += $(TOP)/private/telkom/mwifidogvalidate.o
endif

ifeq ($(SAMBA_SUPPORT),1)
OBJS  += samba.o
endif
ifeq ($(CONFIG_SAMBA),y)
CFLAGS += -DHAVE_SAMBA
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS += -DHAVE_JFFS2
endif
ifeq ($(CONFIG_MMC),y)
CFLAGS += -DHAVE_MMC
endif
#ifeq ($(HW_QOS_SUPPORT),1)
#OBJS  += qos.o
#endif
ifeq ($(PERFORMANCE_SUPPORT),1)
OBJS  += performance.o
endif
#ifeq ($(WL_STA_SUPPORT),1)
VISOBJS  += site_survey.o
#endif

ifeq ($(CONFIG_PPPOESERVER),y)
CFLAGS += -DHAVE_PPPOESERVER
VISOBJS += chaps.o
endif

#ifeq ($(CONFIG_MILKFISH),y)
#CFLAGS += -DHAVE_MILKFISH
#VISOBJS += milkfish.o
#endif

ifeq ($(CONFIG_RT2880),y)
ifneq ($(CONFIG_MT76),y)
  CFLAGS += -DHAVE_RT2880
  VISOBJS += wireless_ralink.o
endif
endif
ifeq ($(CONFIG_RT61),y)
  CFLAGS += -DHAVE_RT2880
  CFLAGS += -DHAVE_RT61
  VISOBJS += wireless_ralink.o
endif
ifeq ($(CONFIG_RT3052),y)
  CFLAGS += -DHAVE_RT2880  -DHAVE_RT3052
endif
ifeq ($(CONFIG_WHRG300N),y)
  CFLAGS += -DHAVE_WHRG300N
endif
ifeq ($(CONFIG_WR5422),y)
  CFLAGS += -DHAVE_WR5422
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_WHRG300NV2),y)
CFLAGS += -DHAVE_WHRG300NV2
endif
ifeq ($(CONFIG_WHRHPG300N),y)
CFLAGS += -DHAVE_WHRHPG300N
endif
ifeq ($(CONFIG_CUSTOMSSLCERT),y)
CFLAGS += -DHAVE_CUSTOMSSLCERT
endif



#use this do disable number of arguments checking (less code and faster exec)
CFLAGS += -DFASTWEB

#JSON encoding/decoding
CFLAGS += -I$(TOP)/jansson/src

#OBJS += http.o

ifneq ($(CONFIG_NOSSL),y)
ifeq ($(CONFIG_OPENSSL),y)
ifeq ($(CONFIG_FREERADIUS),y)
CONFIG_MATRIXSSL=n
endif
ifeq ($(CONFIG_ASTERISK),y)
CONFIG_MATRIXSSL=n
endif
ifeq ($(CONFIG_AIRCRACK),y)
CONFIG_MATRIXSSL=n
endif
#ifeq ($(CONFIG_POUND),y)
#CONFIG_MATRIXSSL=n
#endif
ifeq ($(CONFIG_IPETH),y)
CONFIG_MATRIXSSL=n
endif
ifeq ($(CONFIG_WPA3),y)
CONFIG_MATRIXSSL=n
endif
ifeq ($(CONFIG_VPNC),y)
CONFIG_MATRIXSSL=n
endif
ifeq ($(CONFIG_TOR),y)
CONFIG_MATRIXSSL=n
endif
endif
ifeq ($(CONFIG_POLARSSL),y)
CONFIG_MATRIXSSL=n
CFLAGS += -I$(TOP)/polarssl/include
SSLLDFLAGS += -L$(TOP)/polarssl/library -lpolarssl -lpthread
CFLAGS += -DHAVE_POLARSSL
CFLAGS += -DHAVE_HTTPS
endif

ifneq ($(CONFIG_MATRIXSSL),y)
ifeq ($(CONFIG_WOLFSSL),y)

ifeq ($(CONFIG_WOLFSSLMIN),y)
CFLAGS += -I$(TOP)/wolfssl -I$(TOP)/wolfssl/minimal  -I$(TOP)/wolfssl/minimal/wolfssl
SSLLDFLAGS += $(LDLTO) -L$(TOP)/wolfssl/minimal/src/.libs -lwolfssl -lm 
else
CFLAGS += -I$(TOP)/wolfssl -I$(TOP)/wolfssl/standard -I$(TOP)/wolfssl/standard/wolfssl
SSLLDFLAGS += $(LDLTO) -L$(TOP)/wolfssl/standard/src/.libs -lwolfssl -lm 
endif
CFLAGS += -I$(TOP)/wolfssl/wolfssl $(LTOMIN)
CFLAGS += -DHAVE_OPENSSL
CFLAGS += -DHAVE_HTTPS
CFLAGS += -DHAVE_WOLFSSL
CFLAGS += -DOPENSSL_EXTRA

else
ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -I$(TOP)/openssl/include  -I$(TOP)/openssl/include/openssl
SSLLDFLAGS += -L$(TOP)/openssl -L$(TOP)/openssl -L$(TOP)/zlib -lssl -lcrypto -L$(TOP)/libucontext -lucontext
CFLAGS += -DHAVE_OPENSSL
CFLAGS += -DHAVE_HTTPS
endif
endif
endif




#ifeq ($(CONFIG_MATRIXSSL),y)
#CONFIG_XYSSL=y
#CFLAGS += -I$(TOP)/matrixssl
#SSLLDFLAGS += -L$(TOP)/matrixssl/src -lmatrixsslstatic -lpthread
#CFLAGS += -DHAVE_MATRIXSSL
#CFLAGS += -DHAVE_HTTPS
#OBJS += matrixssl_xface.o
#endif


endif

#OBJS  += common.o server.o
#OBJS  += code_header.o



vpath %.c modules modules/special.$(ARCH) $(SRCBASE)/../tools/src validate visuals ../private/buffalo ../private/buffalo/intatstart/httpd/validate ../private/buffalo/intatstart/httpd/visuals

%.o : %.c
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

all: generate httpd_static cert

generate:
	@echo "[GENWWW] gen"
	@gcc -o gen gen.c
	@./gen > webs.h

clean:
	rm -f *.o *~ httpd httpd_static
	rm -f modules/*.o
	rm -f modules/*.*~
	rm -f validate/*.*~
	rm -f visuals/*.*~
	rm -f *.a 
	#Added by Daniel(2004-07-29)
	#cp ../shared/ezc.o ./
	#rm -rf $(SRCBASE)/../tools/src/code_header.o	

install:
#	lzma e www www.lz
ifneq ($(CONFIG_LITE),y)
	install -D www $(INSTALLDIR)/etc/www
	install -D httpd_static $(INSTALLDIR)/usr/sbin/httpd
	mkdir -p $(INSTALLDIR)/usr/lib
#	install -D validate.so $(INSTALLDIR)/usr/lib
#	install -D visuals.so $(INSTALLDIR)/usr/lib
ifneq ($(CONFIG_NOSSL),y)
	install -d $(INSTALLDIR)/etc
	install *.pem $(INSTALLDIR)/etc
endif
endif
#	cd $(INSTALLDIR)/usr/sbin && ln -sf httpd httpsclient

cert:
ifeq ($(CONFIG_CARLSONWIRELESS),y)
	./gencert_carlsonwireless.sh
else
ifeq ($(CONFIG_ONNET),y)
	./gencert_onnet.sh
else
ifeq ($(CONFIG_IPR),y)
	./gencert_ipr.sh
else
	./gencert.sh
endif
endif
endif

packets.o: packets.c
	@$(CC) -c -o $@ $^ $(BCFLAGS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/iptables/include  -I$(TOP)/iptables -DIPTCSTATIC=static

	
httpd: httpd.c $(ADDOBJ) $(OBJS)
	@echo "[LD] $@"
#	gcc -o write_web write_web.c
#	./write_web www 
	@$(CC) -fvisibility=hidden -o $@ $^ $(CFLAGS) $(SSLCFLAGS) $(SSLLDFLAGS) $(LDFLAGS) $(LTO) $(LDLTO)
#	ar arc -o $@ $^
	rm -f ../rc/rc

httpd_static: httpd.c $(ADDOBJ) $(OBJS)  validators.o webs.o wepkey.o setupassist.o aoss_save.o wps.o $(WEBSOBJS) $(REGOBJS)  core.o dd-wrt.o packets.o lib.o ejs.o wireless.o getservice.o ddns.o index.o status.o filters.o forward.o static_route.o dynamic_route.o log.o dhcp.o diag.o find_pattern.o setupassistant.o gps.o aoss.o ejwps.o fs_common.o status_gpio.o $(VISOBJS) $(REGOBJS2)

	@echo "[LD] $@"
ifeq ($(CONFIG_SUPERCHANNEL),y)
	cp $(REGOBJS2) .
endif
ifeq ($(CONFIG_REGISTER),y)
	cp $(REGOBJS2) .
endif
ifeq ($(CONFIG_ATH9K),y)
	@$(CC) -c -o $(ATH9KVISOBJS) $(ATH9KVISFILES) $(CFLAGS) -DVISUALSOURCE $(LDFLAGS)
endif
	gcc -o makeej makeej.c
	gcc-nm *.o | grep ej_| awk '{print $$3}' > ejsyms
	cp www www.renew
	cat ej_temp/* >> www.renew
	./makeej > modules/ejtable.h
#	gcc -o write_web write_web.c
#	./write_web www 
#	@$(CC) -c $^ -DSTATIC_BUILD $(CFLAGS) $(SSLCFLAGS) $(SSLLDFLAGS) $(ATH9KVISOBJS) $(LDFLAGS)  $(WEBSLDFLAGS) $(LTO) $(LDLTO)
	@$(CC) -o $@ $^ -DSTATIC_BUILD $(CFLAGS) $(SSLCFLAGS) $(SSLLDFLAGS) $(ATH9KVISOBJS) $(LDFLAGS)  $(WEBSLDFLAGS) $(LTO) $(LDLTO)
#	ar arc -o $@ $^
	rm -f ../rc/rc

validate.so: validators.o webs.o wepkey.o setupassist.o aoss_save.o wps.o $(WEBSOBJS) $(REGOBJS)
	@echo "[LD] $@"
	@$(CC) -fvisibility=hidden --shared -o $@ $^ $(CFLAGS) -DVALIDSOURCE -DGETREGCODE $(LDFLAGS) $(WEBSLDFLAGS)

visuals.so: dd-wrt.o packets.o lib.o ejs.o wireless.o getservice.o ddns.o index.o status.o filters.o forward.o static_route.o dynamic_route.o log.o dhcp.o diag.o find_pattern.o setupassistant.o gps.o aoss.o ejwps.o fs_common.o status_gpio.o $(VISOBJS) $(REGOBJS2)
	@echo "[LD] $@"
ifeq ($(CONFIG_ATH9K),y)
	@$(CC) -fvisibility=hidden -c -o $(ATH9KVISOBJS) $(ATH9KVISFILES) $(CFLAGS) -DVISUALSOURCE $(LDFLAGS)
endif
	@$(CC) -fvisibility=hidden --shared -o $@ $^ $(NVISOBJS) $(ATH9KVISOBJS) $(CFLAGS) -DVISUALSOURCE $(LDFLAGS) $(WEBSLDFLAGS) $(LDLTO)
	
	
build_date.o: build_date.c
build_date:
	echo "#define BUILD_DATE \"`date \"+%b %d %Y\"`\"" > build_date.c
	echo "#define BUILD_TIME \"`date \"+%H:%M:%S\"`\"" >> build_date.c


*.o: $(CY_DEPS)
