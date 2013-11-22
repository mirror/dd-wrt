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
CFLAGS += -DNDEBUG -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
CFLAGS += -I$(SRCBASE)/include.v24 
endif

CFLAGS += $(MIPS16_OPT) 


#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -fPIC 
LDFLAGS += -L.
OBJS := utils.o 
ifeq ($(CONFIG_RT2880),y)
OBJS += utils.o crypt.save des.save md5.save
endif
OBJS += shutils.o wl_basic.o linux_timer.o wl_linux.o gpio.o md5c.o crc.o sha1.o nvparse.o bcmutils.o linux_osl.o backup.o
WOBJS := wl.o wl_macfuncs.o

#defaults.o
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -fpic
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -fpic
endif
ifeq ($(CONFIG_SUPERCHANNEL),y)
CFLAGS += -DHAVE_SUPERCHANNEL
ATH9KCFLAGS += -DHAVE_SUPERCHANNEL
OBJS += $(TOP)/register/register_check.o
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
ATH9KCFLAGS += -DHAVE_REGISTER
OBJS += $(TOP)/register/register_check.o
endif
ifeq ($(CONFIG_MADWIFI),y)
OBJS += $(TOP)/register/register_check.o
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
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -I../services/networking -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL -DATH_USE_COMMONHAL
NCFLAGS += $(COPTS) -fPIC -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -I../services/networking -I../madwifi.dev/madwifi_mimo.dev/core  -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL -DATH_USE_COMMONHAL
NCFLAGS	+= -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
NOBJS += wl_11n.o
NFILES += wl_11n.c
endif
ifeq ($(CONFIG_ATH9K),y)
ATH9KCFLAGS+= $(COPTS) -fPIC -DHAVE_ATH9K -I. -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v24 -I$(TOP)/compat-wireless/include -I$(TOP)/compat-wireless/include/uapi/ -I$(TOP)/libnl-tiny/include -I$(SRCBASE)/
ATH9KOBJS += wl_ath9k.o list_sort.o unl.o mac80211info.o mac80211regulatory.o mac80211autochannel.o
ATH9KFILES += wl_ath9k.c list_sort.c unl.c mac80211info.c mac80211regulatory.c mac80211autochannel.c
ATH9KLDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
endif


ifeq ($(CONFIG_MADWIFI),y)
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -I../services/networking -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL -DATH_USE_COMMONHAL
else
CFLAGS += -DHAVE_MADWIFI -I../services/networking -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL 
endif
WLDFLAGS = -L $(TOP)/wireless-tools -liw
else
WLDFLAGS = -lutils
endif
OBJS += country.o


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
ifeq ($(CONFIG_NORTHSTAR),y)
  CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_CARAMBOLA),y)
  CFLAGS += -DHAVE_CARAMBOLA
endif
ifeq ($(CONFIG_NEXTMEDIA),y)
  CFLAGS += -DHAVE_NEXTMEDIA
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
ifeq ($(CONFIG_SAMBA3),y)
  OBJS += samba3.o 
  CFLAGS += -I$(TOP)/jansson/src -L$(TOP)/jansson/src/.libs -ljansson -lm
  CFLAGS += -DHAVE_SAMBA3
  CFLAGS += -DHAVE_SAMBA_SERVER
endif
ifeq ($(CONFIG_FTP),y)
  OBJS += samba3.o 
  CFLAGS += -I$(TOP)/jansson/src -L$(TOP)/jansson/src/.libs -ljansson -lm
endif
ifeq ($(CONFIG_MINIDLNA),y)
  OBJS +=  dlna.o
  CFLAGS += -I$(TOP)/jansson/src -L$(TOP)/jansson/src/.libs -ljansson -lm
  CFLAGS += -DHAVE_MINIDLNA
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

ifeq ($(CONFIG_RAMSKOV),y)
  CFLAGS += -DHAVE_RAMSKOV
endif

ifeq ($(CONFIG_SNMP),y)
  CFLAGS += -DHAVE_SNMP
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
ifeq ($(CONFIG_MT7620),y)
  CFLAGS += -DHAVE_MT7620
endif

ifeq ($(CONFIG_XSCALE),y)
  CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_VF803),y)
  CFLAGS += -DHAVE_VF803
endif
ifeq ($(CONFIG_SX763),y)
  CFLAGS += -DHAVE_SX763
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
ifeq ($(CONFIG_3G),y)
  CFLAGS += -DHAVE_3G
  OBJS += lib3g.o
endif
ifeq ($(CONFIG_LIBQMI),y)
  CFLAGS += -DHAVE_LIBQMI
endif
ifeq ($(CONFIG_UQMI),y)
  CFLAGS += -DHAVE_UQMI
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
ifeq ($(CONFIG_RB800),y)
  CFLAGS += -DHAVE_RB800
endif
ifeq ($(CONFIG_RB1000),y)
  CFLAGS += -DHAVE_RB1000
endif
ifeq ($(CONFIG_FREERADIUS),y)
  CFLAGS += -DHAVE_FREERADIUS
  OBJS += radius.o
endif
ifeq ($(CONFIG_POKER),y)
  CFLAGS += -DHAVE_POKER
  OBJS += poker.o
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
ifeq ($(CONFIG_IPR),y)
  CFLAGS += -DHAVE_IPR
endif
ifeq ($(CONFIG_ONNET),y)
  CFLAGS += -DHAVE_ONNET
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

ifeq ($(CONFIG_CHILLILOCAL),y)
  CFLAGS += -DHAVE_CHILLILOCAL
endif

ifeq ($(CONFIG_PPPOERELAY),y)
  CFLAGS += -DHAVE_PPPOERELAY
endif
ifeq ($(CONFIG_PPPOATM),y)
  CFLAGS += -DHAVE_PPPOATM
endif

ifeq ($(CONFIG_CHILLISPOT),y)
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
ifeq ($(CONFIG_WR841V8),y)
CFLAGS += -DHAVE_WR841V8
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
ifeq ($(CONFIG_WR741),y)
CFLAGS += -DHAVE_WR741
endif
ifeq ($(CONFIG_WA7510),y)
CFLAGS += -DHAVE_WA7510
endif
ifeq ($(CONFIG_WR703),y)
CFLAGS += -DHAVE_WR703
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

ifeq ($(CONFIG_IMMERSIVE),y)
CFLAGS += -DHAVE_IMMERSIVE
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
CFLAGS += -DCONFIG_BRANDING
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
HFLAGS := $(CFLAGS)
CFLAGS += $(COPTS)
all: libutils.so libutils.a libwireless.so libwireless.a

install: all
ifeq ($(CONFIG_XSCALE),y)
	install -d $(INSTALLDIR)/libutils/lib
	install -m 755 libutils.so $(INSTALLDIR)/libutils/lib
	install -m 755 libwireless.so $(INSTALLDIR)/libutils/lib
else
	install -d $(INSTALLDIR)/libutils/usr/lib
	install -m 755 libutils.so $(INSTALLDIR)/libutils/usr/lib
	install -m 755 libwireless.so $(INSTALLDIR)/libutils/usr/lib
endif

clean:
#	rm -f countrycode/countrycode_$(ARCH).o
	rm -f *.o *.so
	rm -f *.a
	rm -f *.h~
	rm -f *.c~
	rm -f $(TOP)/private/telkom/*.o



libutils.so: $(OBJS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^  -lcrypt -L../nvram -lnvram

mchillispot.o: $(TOP)/private/telkom/mchillispot.c
	$(CC) -DNEED_PRINTF -I$(TOP)/jansson/src $(ATH9KCFLAGS) $(MIPS16_OPT) -c -o $@ $^

libwireless.so: $(WOBJS)
ifeq ($(CONFIG_MADWIFI_MIMO),y)
	$(CC) -shared -c $(NCFLAGS) $(MIPS16_OPT) $(NFILES)
endif
ifeq ($(CONFIG_ATH9K),y)
	$(CC) -shared -c $(ATH9KCFLAGS) $(MIPS16_OPT) $(ATH9KFILES) 
endif
	$(CC) -shared $(CFLAGS) $(LDFLAGS)  -o $@ $^ $(NOBJS) $(ATH9KOBJS) -L../nvram -lnvram  $(WLDFLAGS) $(ATH9KLDFLAGS)


libutils.a: $(OBJS)
	$(AR) cr $@ $^

libwireless.a: $(WOBJS)
	$(AR) cr $@ $^

*.o: $(CY_DEPS)
