-include $(LINUXDIR)/.config
CONFIG_WR941=
CONFIG_WA901=
CONFIG_DIR615I=
CONFIG_RB2011=
CONFIG_WR741=
CONFIG_WZRAG300NH=
CONFIG_DIR615E=
CONFIG_WP543=
CONFIG_WDR4300=
CONFIG_WDR2543=
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

COPTS += $(MIPS16_OPT) 

BCFLAGS	= $(COPTS)  -fPIC -DMEMLEAK_OVERRIDE

ifeq ($(CONFIG_80211AC),y)
BCFLAGS	+= -I. -I$(LINUXDIR)/brcm/$(ARCH)/include -Wall
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
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(TOP)/libutils -lcrypt -ldl -lutils -lwireless
#LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(TOP)/libutils -lcrypt -ldl -lutils -lwireless -L$(TOP)/jansson/src/.libs/ -ljansson

BCFLAGS += -DLINUX -ffunction-sections -fdata-sections -Wl,--gc-sections

CFLAGS = $(BCFLAGS) -I$(LINUXDIR)/brcm/shared/bcmwifi/include -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include

REWRITE_REVISION_H1 := $(shell echo -n '\#define SVN_REVISION "' > revision.h)
REWRITE_REVISION_H2 := $(shell svnversion -n . >> revision.h)
REWRITE_REVISION_H3 := $(shell echo '"' >> revision.h)

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
ifeq ($(CONFIG_80211AC),y)
CFLAGS += -DHAVE_80211AC
endif
ifeq ($(CONFIG_WZRG450),y)
CFLAGS += -DHAVE_WZRG450
endif
ifeq ($(CONFIG_WZRG300NH2),y)
CFLAGS += -DHAVE_WZRG300NH2
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
ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DHAVE_BCMMODERN
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS += -DHAVE_SYSLOG
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
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
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
ifeq ($(CONFIG_FREECWMP),y)
CFLAGS += -DHAVE_FREECWMP
VISOBJS += freecwmp.o
endif
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
OBJS += upgrade_rb500.o
else
ifeq ($(CONFIG_WDR4900),y)
OBJS += upgrade.o
else


ifneq ($(CONFIG_X86),y)
ifneq ($(CONFIG_RB600),y)
ifeq ($(CONFIG_BUFFALO),y)
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
OBJS += upgrade_buffalo.o
else
OBJS += upgrade.o
endif
else
OBJS += upgrade.o
endif
CFLAGS += -DHAVE_LOUD
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
  CFLAGS += -DHAVE_LAGUNA
  CFLAGS += -DHAVE_CPUTEMP
  CFLAGS += -DHAVE_VOLT
endif
ifeq ($(CONFIG_VENTANA),y)
  CFLAGS += -DHAVE_VENTANA
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
ifeq ($(CONFIG_IPETH),y)
  CFLAGS += -DHAVE_IPETH
endif
ifeq ($(CONFIG_RTG32),y)
  CFLAGS += -DHAVE_RTG32
endif

ifeq ($(CONFIG_XSCALE),y)
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
ifeq ($(CONFIG_ERC),y)
CFLAGS += -DHAVE_ERC
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
ifeq ($(CONFIG_VILIM),y)
CFLAGS += -DHAVE_VILIM
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
CFLAGS += -DHAVE_X86
OBJS += upgrade_x86.o
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -DHAVE_RB600

ifneq ($(CONFIG_WDR4900),y)
OBJS += upgrade_x86.o
endif
endif

ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
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
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
CFLAGS += -DHAVE_CHILLILOCAL
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif
ifeq ($(CONFIG_DLM101),y)
  CFLAGS += -DHAVE_DLM101
endif

ifeq ($(CONFIG_FTP),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
CFLAGS += -DHAVE_NAS_SERVER
endif

ifeq ($(CONFIG_SAMBA3),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs 
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
CFLAGS += -DHAVE_NAS_SERVER
CFLAGS += -DHAVE_SAMBA_SERVER
endif

ifeq ($(CONFIG_MINIDLNA),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
CFLAGS += -DHAVE_MINIDLNA
endif

ifeq ($(CONFIG_PRIVOXY),y)
CFLAGS += -DHAVE_PRIVOXY
endif

ifeq ($(CONFIG_LIGHTTPD),y)
CFLAGS += -DHAVE_LIGHTTPD
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
VISOBJS += sputnik.o
endif
ifeq ($(CONFIG_USB),y)
CFLAGS += -DHAVE_USB
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
#ifeq ($(HSIAB_SUPPORT),1)
#CFLAGS += -I$(TOP)/hsiab
#LDFLAGS += -L$(TOP)/openssl -L$(TOP)/hsiab -lcrypto -lssl -lhsiab
#OBJS += hsiab.o
#endif
#ifeq ($(HTTPS_SUPPORT),1)
#CFLAGS += -I$(TOP)/openssl/include  -I$(TOP)/openssl/include/openssl
#LDFLAGS += -L$(TOP)/openssl -L../openssl -lcrypto -lssl
#endif
#ifeq ($(BACKUP_RESTORE_SUPPORT),1)
#OBJS  += $(SRCBASE)/../tools/src/code_header.o backup_restore.o
#endif

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

ifeq ($(CONFIG_OPENVPN),y)
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
endif
endif
ifeq ($(ARCHITECTURE),northstar)
ifneq ($(CONFIG_MADWIFI),y)
VISOBJS += wireless_broadcom.o
endif
endif



ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -DHAVE_ACK -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -DATH_USE_COMMONHAL
ifneq ($(ARCHITECTURE),broadcom)
CFLAGS += -DHAVE_RELAYD
endif
NCFLAGS += $(BCFLAGS) -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -DHAVE_ACK -I../madwifi.dev/madwifi_mimo.dev/core -include ../madwifi.dev/madwifi_mimo.dev/core/include/compat.h  -DATH_USE_COMMONHAL
NVISOBJS += wireless_madwifi11n.o
NVISFILES += visuals/wireless_madwifi11n.c
endif

ifeq ($(CONFIG_ATH9K),y)
LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
ATH9KVISOBJS += wireless_madwifiath9k.o
ATH9KVISFILES += visuals/wireless_madwifiath9k.c
endif


ifeq ($(CONFIG_MADWIFI),y)
ifneq ($(ARCHITECTURE),broadcom)
CFLAGS += -DHAVE_RELAYD
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -DHAVE_ACK -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -DATH_USE_COMMONHAL
VISOBJS += wireless_madwifi.o
else
CFLAGS += -DHAVE_MADWIFI -DHAVE_ACK -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h
VISOBJS += wireless_madwifi.o
endif

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
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_EASY_WIRELESS_CONFIG
endif
ifeq ($(CONFIG_MADWIFI_ATH9K),y)
CFLAGS += -DHAVE_EASY_WIRELESS_CONFIG
endif

ifeq ($(CONFIG_IFL),y)
CFLAGS += -DHAVE_IFL
endif

ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
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

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
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
CFLAGS += -DCONFIG_BRANDING
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
ifeq ($(CONFIG_LSX),y)
CFLAGS += -DHAVE_LSX
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS += -DHAVE_WZRG300NH
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
ifeq ($(CONFIG_HORNET),y)
CFLAGS += -DHAVE_HORNET
endif
ifeq ($(CONFIG_WASP),y)
CFLAGS += -DHAVE_WASP
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
ifeq ($(CONFIG_TIEXTRA1),y)
CFLAGS += -DHAVE_TIEXTRA1 -Wl,-rpath,$(TOP)/jansson/src/.libs
LDFLAGS +=   -Wl,-rpath,$(TOP)/jansson/src/.libs
VISOBJS  += $(TOP)/private/telkom/mchillispotejs.o
WEBSOBJS  += $(TOP)/private/telkom/mchillispotvalidate.o
endif
ifeq ($(CONFIG_TIEXTRA2),y)
CFLAGS += -DHAVE_TIEXTRA2 -Wl,-rpath,$(TOP)/jansson/src/.libs
LDFLAGS +=   -Wl,-rpath,$(TOP)/jansson/src/.libs
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

ifeq ($(CONFIG_MILKFISH),y)
CFLAGS += -DHAVE_MILKFISH
VISOBJS += milkfish.o
endif

ifeq ($(CONFIG_RT2880),y)
  CFLAGS += -DHAVE_RT2880
  VISOBJS += wireless_ralink.o
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

#use this do disable number of arguments checking (less code and faster exec)
CFLAGS += -DFASTWEB

#JSON encoding/decoding
CFLAGS += -I$(TOP)/jansson/src

#OBJS += http.o

ifneq ($(CONFIG_NOSSL),y)

ifneq ($(CONFIG_MATRIXSSL),y)
ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -I$(TOP)/openssl/include  -I$(TOP)/openssl/include/openssl
SSLLDFLAGS += -L$(TOP)/openssl -L$(TOP)/openssl -lcrypto -lssl
CFLAGS += -DHAVE_OPENSSL
CFLAGS += -DHAVE_HTTPS
endif
endif

ifeq ($(CONFIG_MATRIXSSL),y)
#CONFIG_XYSSL=y

CFLAGS += -I$(TOP)/matrixssl
SSLLDFLAGS += -L$(TOP)/matrixssl/src -lmatrixsslstatic -lpthread
CFLAGS += -DHAVE_MATRIXSSL
CFLAGS += -DHAVE_HTTPS
OBJS += matrixssl_xface.o
endif

ifeq ($(CONFIG_XYSSL),y)
CFLAGS += -I$(TOP)/httpd/xyssl/include
SSLLDFLAGS += -L$(TOP)/httpd/xyssl/library -lxyssl -lpthread
CFLAGS += -DHAVE_XYSSL
CFLAGS += -DHAVE_HTTPS
endif

endif

#OBJS  += common.o server.o
#OBJS  += code_header.o



vpath %.c modules modules/special.$(ARCH) $(SRCBASE)/../tools/src validate visuals ../private/buffalo ../private/buffalo/intatstart/httpd/validate ../private/buffalo/intatstart/httpd/visuals


all: generate validate.so visuals.so httpd cert

generate:
	gcc -o gen gen.c
	./gen > webs.h

clean:
	cd xyssl && make clean
	rm -f *.o *~ httpd
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
	install -D www $(INSTALLDIR)/etc/www
	install -D httpd $(INSTALLDIR)/usr/sbin/httpd
	mkdir -p $(INSTALLDIR)/usr/lib
	install -D validate.so $(INSTALLDIR)/usr/lib
	install -D visuals.so $(INSTALLDIR)/usr/lib
ifneq ($(CONFIG_NOSSL),y)
	install -d $(INSTALLDIR)/etc
	install *.pem $(INSTALLDIR)/etc
endif
ifeq ($(CONFIG_IAS),y)
	install -d $(INSTALLDIR)/etc/config
ifeq ($(CONFIG_IAS_NOPLNPWD),y)
	install ../private/buffalo/intatstart/config/admincard_noplnpwd.webdisplay $(INSTALLDIR)/etc/config/admincard.webdisplay
else
	install ../private/buffalo/intatstart/config/admincard.webdisplay $(INSTALLDIR)/etc/config/admincard.webdisplay
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
	
httpd: cgi.o ej.o httpd.o broadcom.o \
       $(ADDOBJ) nvramsr.o callvalidate.o $(OBJS)
#	gcc -o write_web write_web.c
ifeq ($(CONFIG_XYSSL),y)
	cd xyssl && make static
endif
#	./write_web www 
	$(CC) -o $@ $^ $(CFLAGS) $(SSLCFLAGS) $(SSLLDFLAGS) $(LDFLAGS)
#	ar arc -o $@ $^
	rm -f ../rc/rc

validate.so: validators.o webs.o wepkey.o setupassist.o aoss_save.o wps.o $(WEBSOBJS) $(REGOBJS)
	$(CC) --shared -o $@ $^ $(CFLAGS) -DVALIDSOURCE -DGETREGCODE $(LDFLAGS)

visuals.so: dd-wrt.o lib.o ejs.o wireless.o getservice.o ddns.o index.o status.o filters.o forward.o static_route.o dynamic_route.o log.o dhcp.o diag.o find_pattern.o setupassistant.o gps.o aoss.o ejwps.o $(VISOBJS) $(REGOBJS2)
ifeq ($(CONFIG_MADWIFI_MIMO),y)
	$(CC) --shared -c -o $(NVISOBJS) $(NVISFILES) $(NCFLAGS) -DVISUALSOURCE $(LDFLAGS)
endif
ifeq ($(CONFIG_ATH9K),y)
	$(CC) --shared -c -o $(ATH9KVISOBJS) $(ATH9KVISFILES) $(CFLAGS) -DVISUALSOURCE $(LDFLAGS)
endif
	$(CC) --shared -o $@ $^ $(NVISOBJS) $(ATH9KVISOBJS) $(CFLAGS) -DVISUALSOURCE $(LDFLAGS)
	
	
build_date.o: build_date.c
build_date:
	echo "#define BUILD_DATE \"`date \"+%b %d %Y\"`\"" > build_date.c
	echo "#define BUILD_TIME \"`date \"+%H:%M:%S\"`\"" >> build_date.c


*.o: $(CY_DEPS)
