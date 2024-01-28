include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif



ifeq ($(CONFIG_80211AC),y)
CFLAGS	= -I. -I$(LINUXDIR)/brcm/$(ARCH)/include -Wall
CFLAGS	= -I. -I$(LINUXDIR)/brcm/$(ARCH)/common/include
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
endif
else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN -I$(LINUXDIR)/brcm/shared/bcmwifi/include -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include 
CFLAGS  += $(COPTS) 
# -Werror=maybe-uninitialized -Werror=int-conversion -Werror=array-bounds
LDFLAGS	+= -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/libutils -lutils -ldl
CFLAGS += $(LTO) -I$(LINUXDIR) -I$(TOP)/libutils/include -D_GNU_SOURCE
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += $(MIPS16_OPT) $(LERROR)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)
CFLAGS +=$(call cc-option,-Werror=return-local-addr,)


INITCFLAGS = $(call cc-option,-ffunction-sections -fdata-sections,)
# -fno-guess-branch-probability: prohibit pseudo-random guessing
# of branch probabilities (hopefully makes bloatcheck more stable):
#INITCFLAGS += $(call cc-option,-fno-guess-branch-probability,)
#INITCFLAGS += $(call cc-option,-funsigned-char -static-libgcc,)
#INITCFLAGS += $(call cc-option,-falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1,)

#LDFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections

OBJS := rc.o
#http.o stats.o

#ifdef $(CONFIG_DIST)
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"
#endif

CFLAGS += -DARCHITECTURE="\"$(ARCHITECTURE)\""

ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
CFLAGS += -DNEED_NOPRINTF
endif
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
CFLAGS += -DNEED_NOPRINTF
endif
ifeq ($(CONFIG_DNSMASQ),y)
CFLAGS += -DHAVE_DNSMASQ
endif
ifeq ($(CONFIG_SMARTDNS),y)
CFLAGS += -DHAVE_SMARTDNS
endif
ifeq ($(CONFIG_DNSCRYPT),y)
CFLAGS += -DHAVE_DNSCRYPT
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif
ifeq ($(CONFIG_RSTATS),y)
CFLAGS += -DHAVE_RSTATS
endif
ifeq ($(CONFIG_ECB9750),y)
CFLAGS += -DHAVE_ECB9750
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS += -DHAVE_SYSLOG
endif
ifeq ($(CONFIG_SPEEDCHECKER),y)
CFLAGS += -DHAVE_SPEEDCHECKER
endif
ifeq ($(CONFIG_OVERCLOCKING),y)
CFLAGS += -DHAVE_OVERCLOCKING
endif
ifeq ($(CONFIG_80211AC),y)
  CFLAGS += -DHAVE_80211AC
  CFLAGS += -DHAVE_EMF
endif
ifeq ($(CONFIG_NORTHSTAR),y)
  CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_QTN),y)
  CFLAGS += -DHAVE_QTN -DRPCOVERRIDE
  CFLAGS +=  -I$(TOP)/qtn/libqcsapi_client -I$(TOP)/qtn/libqcsapi_client/librpc    
endif
ifeq ($(CONFIG_ACK),y)
CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS += -DHAVE_JFFS2
endif
ifeq ($(CONFIG_WR650AC),y)
CFLAGS += -DHAVE_WR650AC
endif
ifeq ($(CONFIG_E355AC),y)
CFLAGS += -DHAVE_E355AC
endif
ifeq ($(CONFIG_AP120C),y)
CFLAGS += -DHAVE_AP120C
endif
ifeq ($(CONFIG_3G),y)
CFLAGS += -DHAVE_3G
endif
ifeq ($(CONFIG_WR841HPV3),y)
CFLAGS += -DHAVE_WR841HPV3
endif
ifeq ($(CONFIG_E380AC),y)
CFLAGS += -DHAVE_E380AC
endif
ifeq ($(CONFIG_E325N),y)
CFLAGS += -DHAVE_E325N
endif
ifeq ($(CONFIG_SMARTMONTOOLS),y)
CFLAGS += -DHAVE_SMARTD
endif
ifeq ($(CONFIG_XD9531),y)
CFLAGS += -DHAVE_XD9531
endif
ifeq ($(CONFIG_WR615N),y)
CFLAGS += -DHAVE_WR615N
endif
ifeq ($(CONFIG_BRCMFMAC),y)
CFLAGS += -DHAVE_BRCMFMAC
endif
ifeq ($(CONFIG_MI424WR),y)
CFLAGS += -DHAVE_MI424WR
endif
ifeq ($(CONFIG_NP25G),y)
  CFLAGS += -DHAVE_NP25G
endif
ifeq ($(CONFIG_WPE53G),y)
  CFLAGS += -DHAVE_WPE53G
endif
ifeq ($(CONFIG_VNCREPEATER),y)
CFLAGS += -DHAVE_VNCREPEATER
endif
ifeq ($(CONFIG_3G),y)
CFLAGS += -DHAVE_3G
endif
ifeq ($(CONFIG_LIBQMI),y)
CFLAGS += -DHAVE_LIBQMI
endif
ifeq ($(CONFIG_UQMI),y)
CFLAGS += -DHAVE_UQMI
endif
ifeq ($(CONFIG_PERU),y)
CFLAGS += -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox  -I$(TOP)/libnl-tiny/include
CFLAGS += -DHAVE_PERU
endif
ifeq ($(CONFIG_LIMA),y)
CFLAGS += -DHAVE_LIMA
endif
ifeq ($(CONFIG_WR1043V4),y)
CFLAGS += -DHAVE_WR1043V4
endif
ifeq ($(CONFIG_WR1043V5),y)
CFLAGS += -DHAVE_WR1043V5
endif
ifeq ($(CONFIG_PLEX),y)
CFLAGS += -DHAVE_PLEX
endif
ifeq ($(CONFIG_RAMBUTAN),y)
CFLAGS += -DHAVE_RAMBUTAN
endif
ifeq ($(CONFIG_DW02_412H),y)
CFLAGS += -DHAVE_DW02_412H
endif
ifeq ($(CONFIG_SOLO51),y)
CFLAGS += -DHAVE_SOLO51
endif
ifeq ($(CONFIG_KAID),y)
CFLAGS += -DHAVE_KAID
endif
ifeq ($(CONFIG_WILLY),y)
CFLAGS += -DHAVE_WILLY
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_VLANTAGGING
endif
ifeq ($(CONFIG_NSTX),y)
CFLAGS += -DHAVE_NSTX
endif
ifeq ($(CONFIG_GLAUCO),y)
CFLAGS += -DHAVE_GLAUCO
endif
ifeq ($(CONFIG_USB),y)
CFLAGS += -DHAVE_USB
endif
ifeq ($(CONFIG_3G_ONLY),y)
CFLAGS += -DHAVE_3G_ONLY
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_WNR2200),y)
CFLAGS += -DHAVE_WNR2200
endif
ifeq ($(CONFIG_WNR2000),y)
CFLAGS += -DHAVE_WNR2000
endif
ifeq ($(CONFIG_HORNET),y)
CFLAGS += -DHAVE_HORNET
endif
ifeq ($(CONFIG_CARAMBOLA),y)
CFLAGS += -DHAVE_CARAMBOLA
ifeq ($(CONFIG_FMS2111),y)
CFLAGS += -DHAVE_FMS2111 -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox  -I$(TOP)/libnl-tiny/include
endif
endif
ifeq ($(CONFIG_WASP),y)
CFLAGS += -DHAVE_WASP
endif
ifeq ($(CONFIG_DIR825C1),y)
CFLAGS += -DHAVE_DIR825C1
endif
ifeq ($(CONFIG_DIR859),y)
CFLAGS += -DHAVE_DIR859
endif
ifeq ($(CONFIG_DIR869),y)
CFLAGS += -DHAVE_DIR869
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
ifeq ($(CONFIG_DAP2680),y)
CFLAGS += -DHAVE_DAP2680
endif
ifeq ($(CONFIG_DIR862),y)
CFLAGS += -DHAVE_DIR862
endif
ifeq ($(CONFIG_MMS344),y)
CFLAGS += -DHAVE_MMS344
endif
ifeq ($(CONFIG_WR1043V2),y)
CFLAGS += -DHAVE_WR1043V2
endif
ifeq ($(CONFIG_AC622),y)
  CFLAGS += -DHAVE_AC622
endif
ifeq ($(CONFIG_AC722),y)
  CFLAGS += -DHAVE_AC722
endif
ifeq ($(CONFIG_RT61),y)
  CFLAGS += -DHAVE_RT61
endif
ifeq ($(CONFIG_WZR450HP2),y)
CFLAGS += -DHAVE_WZR450HP2
endif
ifeq ($(CONFIG_RB2011),y)
CFLAGS += -DHAVE_RB2011
endif
ifeq ($(CONFIG_WDR4300),y)
CFLAGS += -DHAVE_WDR4300
endif
ifeq ($(CONFIG_UDHCPD),y)
CFLAGS += -DHAVE_UDHCPD
endif
ifeq ($(CONFIG_WHR300HP2),y)
CFLAGS += -DHAVE_WHR300HP2
endif
ifeq ($(CONFIG_E1700),y)
CFLAGS += -DHAVE_E1700
endif
ifeq ($(CONFIG_WDR4900),y)
CFLAGS += -DHAVE_WDR4900
endif
ifeq ($(CONFIG_MVEBU),y)
CFLAGS += -DHAVE_MVEBU
endif
ifeq ($(CONFIG_IPQ806X),y)
CFLAGS += -DHAVE_IPQ806X
endif
ifeq ($(CONFIG_QCA4019),y)
CFLAGS += -DHAVE_QCA4019
endif
ifeq ($(CONFIG_HABANERO),y)
CFLAGS += -DHAVE_HABANERO -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox  -I$(TOP)/libnl-tiny/include
endif
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -DHAVE_ALPINE
endif
ifeq ($(CONFIG_ARCHERC7V4),y)
CFLAGS += -DHAVE_ARCHERC7V4
endif
ifeq ($(CONFIG_ARCHERC7V5),y)
CFLAGS += -DHAVE_ARCHERC7V5
endif
ifeq ($(CONFIG_ARCHERA7V5),y)
CFLAGS += -DHAVE_ARCHERA7V5
endif
ifeq ($(CONFIG_WDR2543),y)
CFLAGS += -DHAVE_WDR2543
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
endif

ifeq ($(CONFIG_GEMTEK),y)
CFLAGS += -DHAVE_GEMTEK
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DDEFAULT_COUNTRY_CODE=\"$(CONFIG_DEFAULT_COUNTRYCODE)\"
endif
ifeq ($(CONFIG_BUFFALO_SA),y)
CFLAGS += -DHAVE_BUFFALO_SA
endif
ifeq ($(CONFIG_UNFY),y)
CFLAGS += -DHAVE_UNFY
endif
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
endif
ifeq ($(CONFIG_OCTEON),y)
CFLAGS += -DHAVE_OCTEON
endif
ifeq ($(CONFIG_DHDAP),y)
CFLAGS += -DHAVE_DHDAP
endif
ifeq ($(CONFIG_EROUTER),y)
CFLAGS += -DHAVE_EROUTER
endif
ifeq ($(CONFIG_E200),y)
CFLAGS += -DHAVE_E200
endif
ifeq ($(CONFIG_DIR600),y)
CFLAGS += -DHAVE_DIR600
endif
ifeq ($(CONFIG_RT10N),y)
CFLAGS += -DHAVE_RT10N
endif
ifeq ($(CONFIG_VF803),y)
CFLAGS += -DHAVE_VF803
endif
ifeq ($(CONFIG_SX763),y)
CFLAGS += -DHAVE_SX763
endif
ifeq ($(CONFIG_RT15N),y)
CFLAGS += -DHAVE_RT15N
endif
ifeq ($(CONFIG_LAGUNA),y)
CFLAGS += -DHAVE_LAGUNA
endif
ifeq ($(CONFIG_GPSI),y)
CFLAGS += -DHAVE_GPSI
endif
ifeq ($(CONFIG_VENTANA),y)
  CFLAGS += -DHAVE_VENTANA
endif
ifeq ($(CONFIG_NEWPORT),y)
  CFLAGS += -DHAVE_NEWPORT
endif
ifeq ($(CONFIG_NEPTUNE),y)
CFLAGS += -DHAVE_NEPTUNE
endif
ifeq ($(CONFIG_RUT500),y)
CFLAGS += -DHAVE_RUT500
endif
ifeq ($(CONFIG_RT3352),y)
CFLAGS += -DHAVE_RT3352
endif
ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
CFLAGS += -DHAVE_PPPOEDUAL
endif
ifeq ($(CONFIG_TOR),y)
CFLAGS += -DHAVE_TOR
endif
ifeq ($(CONFIG_UNIWIP),y)
CFLAGS += -DHAVE_UNIWIP
endif
ifeq ($(CONFIG_PPPOATM),y)
CFLAGS += -DHAVE_PPPOATM
endif
ifeq ($(CONFIG_PRIVOXY),y)
CFLAGS += -DHAVE_PRIVOXY
endif
ifeq ($(CONFIG_ZABBIX),y)
CFLAGS += -DHAVE_ZABBIX
endif
ifeq ($(CONFIG_MDNS),y)
CFLAGS += -DHAVE_MDNS
endif
ifeq ($(CONFIG_WEBSERVER),y)
	CFLAGS += -DHAVE_WEBSERVER
endif
ifeq ($(CONFIG_WMBR_G300NH),y)
CFLAGS += -DHAVE_WMBR_G300NH
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
endif
ifeq ($(CONFIG_R9000),y)
CFLAGS += -DHAVE_R9000
endif
ifeq ($(CONFIG_SPOTPASS),y)
CFLAGS += -DHAVE_SPOTPASS
endif
ifeq ($(CONFIG_WNDR3700),y)
CFLAGS += -DHAVE_WNDR3700
endif
ifeq ($(CONFIG_WNDR3700V4),y)
CFLAGS += -DHAVE_WNDR3700V4
endif
ifeq ($(CONFIG_ALFANX),y)
CFLAGS += -DHAVE_ALFANX -DHAVE_WLANLED  -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include  -D_GNU_SOURCE  -I$(TOP)/libubox -I$(TOP)/libnl-tiny/include
endif
ifeq ($(CONFIG_USR8200),y)
CFLAGS += -DHAVE_USR8200
endif
ifeq ($(CONFIG_IPETH),y)
CFLAGS += -DHAVE_IPETH
endif
ifeq ($(CONFIG_WCRGN),y)
CFLAGS += -DHAVE_WCRGN
endif
ifeq ($(CONFIG_HAMEA15),y)
CFLAGS += -DHAVE_HAMEA15
endif
ifeq ($(CONFIG_OLED),y)
CFLAGS += -DHAVE_OLED
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS += -DHAVE_RB600
endif
ifeq ($(CONFIG_OPENRISC),y)
CFLAGS += -DHAVE_OPENRISC
endif
ifeq ($(CONFIG_WZRG300NH),y)
  CFLAGS += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_AOSS),y)
  CFLAGS += -DHAVE_AOSS
endif
ifeq ($(CONFIG_WPS),y)
  CFLAGS += -DHAVE_WPS
endif
ifeq ($(CONFIG_IAS),y)
  CFLAGS += -DHAVE_IAS
endif
ifeq ($(CONFIG_AP_SERV),y)
  CFLAGS += -DHAVE_AP_SERV
endif

ifeq ($(CONFIG_BONDING),y)
CFLAGS += -DHAVE_BONDING
endif
ifeq ($(CONFIG_WIKINGS),y)
CFLAGS += -DHAVE_WIKINGS
endif
ifeq ($(CONFIG_ESPOD),y)
CFLAGS += -DHAVE_ESPOD
endif
ifeq ($(CONFIG_NEXTMEDIA),y)
CFLAGS += -DHAVE_NEXTMEDIA
endif
ifeq ($(CONFIG_RAIEXTRA),y)
CFLAGS += -DHAVE_RAIEXTRA
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
ifeq ($(CONFIG_WIFIDOG),y)
CFLAGS += -DHAVE_WIFIDOG
endif


ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif
ifeq ($(CONFIG_MMC),y)
CFLAGS += -DHAVE_MMC
endif
ifeq ($(CONFIG_PPPOESERVER),y)
CFLAGS += -DHAVE_PPPOESERVER
endif
ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
endif
ifeq ($(CONFIG_MYNETWAY),y)
  CFLAGS += -DHAVE_MYNETWAY
endif
ifeq ($(CONFIG_F5D8235),y)
CFLAGS += -DHAVE_F5D8235
endif
ifeq ($(CONFIG_MAKSAT_BLANK),y)
CFLAGS += -DHAVE_MAKSAT_BLANK
endif
ifeq ($(CONFIG_ERC),y)
CFLAGS += -DHAVE_ERC
endif
ifeq ($(CONFIG_CORENET),y)
CFLAGS += -DHAVE_CORENET
endif
ifeq ($(CONFIG_ESR6650),y)
  CFLAGS += -DHAVE_ESR6650
endif
ifeq ($(CONFIG_EAP9550),y)
  CFLAGS += -DHAVE_EAP9550
endif

ifeq ($(CONFIG_VILIM),y)
CFLAGS += -DHAVE_VILIM
endif
ifeq ($(CONFIG_MULTICAST),y)
CFLAGS += -DHAVE_MULTICAST
endif
ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif
ifeq ($(CONFIG_TRIMAX),y)
  CFLAGS += -DHAVE_TRIMAX
endif

ifeq ($(CONFIG_MEDIASERVER),y)
CFLAGS += -DHAVE_MEDIASERVER
endif

ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif
ifeq ($(CONFIG_JA76PF),y)
CFLAGS += -DHAVE_JA76PF
endif
ifeq ($(CONFIG_JA76PF2),y)
CFLAGS += -DHAVE_JA76PF2
endif
ifeq ($(CONFIG_ALFAAP94),y)
CFLAGS += -DHAVE_ALFAAP94
endif
#ifeq ($(CONFIG_SYSCTL_EDIT),y)
CFLAGS += -DHAVE_SYSCTL_EDIT
#endif
ifeq ($(CONFIG_JWAP003),y)
CFLAGS += -DHAVE_JWAP003
endif
ifeq ($(CONFIG_JWAP606),y)
CFLAGS += -DHAVE_JWAP606
endif
ifeq ($(CONFIG_WR941),y)
CFLAGS += -DHAVE_WR941
endif
ifeq ($(CONFIG_WR741),y)
CFLAGS += -DHAVE_WR741
endif
ifeq ($(CONFIG_WR741V4),y)
CFLAGS += -DHAVE_WR741V4
endif
ifeq ($(CONFIG_GL150),y)
CFLAGS += -DHAVE_GL150
endif
ifeq ($(CONFIG_WR703),y)
CFLAGS += -DHAVE_WR703
endif
ifeq ($(CONFIG_WA701V2),y)
CFLAGS += -DHAVE_WA701V2
endif
ifeq ($(CONFIG_MR3020),y)
CFLAGS += -DHAVE_MR3020
endif
ifeq ($(CONFIG_XD3200),y)
CFLAGS += -DHAVE_XD3200
endif
ifeq ($(CONFIG_SR3200),y)
CFLAGS += -DHAVE_SR3200
endif
ifeq ($(CONFIG_CPE890),y)
CFLAGS += -DHAVE_CPE890 -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox  -I$(TOP)/libnl-tiny/include
endif
ifeq ($(CONFIG_CPE880),y)
CFLAGS += -DHAVE_CPE880 -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox  -I$(TOP)/libnl-tiny/include
endif


ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
CFLAGS += -DHAVE_EOP_TUNNEL
endif
ifeq ($(CONFIG_DDLAN),y)
CFLAGS += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_WRT160NL),y)
CFLAGS += -DHAVE_WRT160NL
endif
ifeq ($(CONFIG_XSCALE),y)
CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_TONZE),y)
CFLAGS += -DHAVE_TONZE
endif
ifeq ($(CONFIG_WG302),y)
CFLAGS += -DHAVE_WG302
endif
ifeq ($(CONFIG_WG302V1),y)
CFLAGS += -DHAVE_WG302V1
endif
ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
CFLAGS += -DHAVE_USB
endif
ifeq ($(CONFIG_NOP8670),y)
CFLAGS += -DHAVE_NOP8670
CFLAGS += -DHAVE_XSCALE
CFLAGS += -DHAVE_GATEWORX
else
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_XSCALE
CFLAGS += -DHAVE_GATEWORX
CFLAGS += -DHAVE_CPUTEMP
endif
endif
ifeq ($(CONFIG_CAMBRIA),y)
CFLAGS += -DHAVE_CAMBRIA
endif
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_CPUTEMP),y)
CFLAGS += -DHAVE_CPUTEMP
endif

ifeq ($(CONFIG_FONERA),y)
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_WRT54G2),y)
CFLAGS += -DHAVE_WRT54G2
endif
ifeq ($(CONFIG_RTG32),y)
CFLAGS += -DHAVE_RTG32
endif
ifeq ($(CONFIG_WRK54G),y)
CFLAGS += -DHAVE_WRK54G
endif
ifeq ($(CONFIG_MERAKI),y)
CFLAGS += -DHAVE_MERAKI
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_EAP3660),y)
  CFLAGS += -DHAVE_EAP3660
endif
ifeq ($(CONFIG_EOC2610),y)
  CFLAGS += -DHAVE_EOC2610
endif
ifeq ($(CONFIG_EOC5610),y)
  CFLAGS += -DHAVE_EOC5610
endif
ifeq ($(CONFIG_EOC1650),y)
  CFLAGS += -DHAVE_EOC1650
endif
ifeq ($(CONFIG_ECB3500),y)
  CFLAGS += -DHAVE_ECB3500
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
ifeq ($(CONFIG_LS5),y)
CFLAGS += -DHAVE_LS5
endif
ifeq ($(CONFIG_WHRAG108),y)
CFLAGS += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_PB42),y)
CFLAGS += -DHAVE_PB42
endif
ifeq ($(CONFIG_LSX),y)
CFLAGS += -DHAVE_LSX
endif
ifeq ($(CONFIG_DIR825),y)
CFLAGS += -DHAVE_DIR825
endif
ifeq ($(CONFIG_WRT400),y)
CFLAGS += -DHAVE_WRT400
endif
ifeq ($(CONFIG_TG2521),y)
CFLAGS += -DHAVE_TG2521
endif
ifeq ($(CONFIG_WR1043),y)
CFLAGS += -DHAVE_WR1043
endif
ifeq ($(CONFIG_P910ND),y)
CFLAGS += -DHAVE_P910ND
endif
ifeq ($(CONFIG_DIR615E),y)
CFLAGS += -DHAVE_DIR615E
endif
ifeq ($(CONFIG_WR841V9),y)
CFLAGS += -DHAVE_WR841V9
endif
ifeq ($(CONFIG_ARCHERC25),y)
CFLAGS += -DHAVE_ARCHERC25
endif
ifeq ($(CONFIG_WR810N),y)
CFLAGS += -DHAVE_WR810N
endif
ifeq ($(CONFIG_WR941V6),y)
CFLAGS += -DHAVE_WR941V6
endif
ifeq ($(CONFIG_DIR615I),y)
CFLAGS += -DHAVE_DIR615I
endif
ifeq ($(CONFIG_WA850RE),y)
CFLAGS += -DHAVE_WA850RE
endif
ifeq ($(CONFIG_DIR615),y)
CFLAGS += -DHAVE_DIR615
endif
ifeq ($(CONFIG_TEW632BRP),y)
CFLAGS += -DHAVE_TEW632BRP
endif
ifeq ($(CONFIG_UBNTM),y)
CFLAGS += -DHAVE_UBNTM -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox -I$(TOP)/libnl-tiny/include
endif
ifeq ($(CONFIG_WA850RE),y)
CFLAGS += -DHAVE_WA850RE -DHAVE_WLANLED -D_GNU_SOURCE  -I$(TOP)/libubox -I$(TOP)/libnl-tiny/include
endif
ifeq ($(CONFIG_DAP3410),y)
CFLAGS += -DHAVE_DAP3410
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS += -DHAVE_WHRHPGN
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
ifeq ($(CONFIG_ADM5120),y)
CFLAGS += -DHAVE_ADM5120
endif
ifeq ($(CONFIG_WP54G),y)
CFLAGS += -DHAVE_WP54G
endif
ifeq ($(CONFIG_NP28G),y)
CFLAGS += -DHAVE_NP28G
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS += -DHAVE_TW6600
endif
ifeq ($(CONFIG_CA8PRO),y)
CFLAGS += -DHAVE_CA8PRO
endif
ifeq ($(CONFIG_CA8),y)
CFLAGS += -DHAVE_CA8
endif
ifeq ($(CONFIG_ALPHA),y)
CFLAGS += -DHAVE_ALPHA
endif
ifeq ($(CONFIG_USR5453),y)
CFLAGS += -DHAVE_USR5453
endif
ifeq ($(CONFIG_JJAP93),y)
CFLAGS += -DHAVE_JJAP93
endif
ifeq ($(CONFIG_WIVIZ),y)
CFLAGS += -DHAVE_WIVIZ
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif
ifeq ($(CONFIG_IPROUTE2),y)
CFLAGS += -DHAVE_IPROUTE2
endif
ifeq ($(CONFIG_GGEW),y)
CFLAGS += -DHAVE_GGEW
CFLAGS += -DHAVE_NEWMEDIA
endif
ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS += -DHAVE_NEWMEDIA
endif
ifeq ($(CONFIG_PPPOERELAY),y)
CFLAGS += -DHAVE_PPPOERELAY
endif
ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
endif

ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_EBTABLES),y)
CFLAGS += -DHAVE_EBTABLES
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_TELNET),y)
CFLAGS += -DHAVE_TELNET
endif
ifeq ($(CONFIG_MACTELNET),y)
CFLAGS += -DHAVE_MACTELNET
endif

ifeq ($(CONFIG_FTP),y)
CFLAGS += -DHAVE_FTP
endif
ifeq ($(CONFIG_SAMBA3),y)
CFLAGS += -DHAVE_SAMBA3
endif
ifeq ($(CONFIG_SMBD),y)
CFLAGS += -DHAVE_SMBD
endif
ifeq ($(CONFIG_NFS),y)
CFLAGS += -DHAVE_NFS
endif
ifeq ($(CONFIG_RSYNC),y)
CFLAGS += -DHAVE_RSYNC
endif
ifeq ($(CONFIG_RAID),y)
CFLAGS += -DHAVE_RAID
endif
ifeq ($(CONFIG_DDNS),y)
CFLAGS += -DHAVE_DDNS
endif
ifeq ($(CONFIG_MINIDLNA),y)
CFLAGS += -DHAVE_MINIDLNA
endif

ifeq ($(CONFIG_SOFTETHER),y)
CFLAGS += -DHAVE_SOFTETHER
endif

ifeq ($(CONFIG_PRIVOXY),y)
CFLAGS += -DHAVE_PRIVOXY
endif

ifeq ($(CONFIG_TRANSMISSION),y)
CFLAGS += -DHAVE_TRANSMISSION
endif

CFLAGS += -DHAVE_AQOS

ifeq ($(CONFIG_PPTPD),y)
CFLAGS += -DHAVE_PPTPD
endif
ifeq ($(CONFIG_PPTP),y)
CFLAGS += -DHAVE_PPTP
endif

ifeq ($(CONFIG_L2TP),y)
CFLAGS += -DHAVE_L2TP
endif
ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS += -DHAVE_HEARTBEAT
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
CFLAGS += -DHAVE_SSHD
#LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(CONFIG_IPV6),y)
CFLAGS += -DHAVE_IPV6
endif
ifeq ($(CONFIG_IPV6),y)
ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
endif
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFWD
endif
ifeq ($(CONFIG_PPPD),y)
CFLAGS += -DHAVE_PPPD
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif
ifeq ($(CONFIG_HOTSPOT),y)
CFLAGS += -DHAVE_HOTSPOT
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

ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
endif

ifeq ($(CONFIG_ZEBRA),y)
CFLAGS += -DHAVE_ZEBRA
endif

ifeq ($(CONFIG_WSHAPER),y)
CFLAGS += -DHAVE_WSHAPER
endif

ifeq ($(CONFIG_SVQOS),y)
CFLAGS += -DHAVE_SVQOS
endif

ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DHAVE_BCMMODERN
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS +=  -DHAVE_MADWIFI -DHAVE_ROAMING 
LDFLAGS +=  -L$(TOP)/wireless-tools
endif
ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS += -DHAVE_NODOG
endif

ifeq ($(CONFIG_LLTD),y)
CFLAGS += -DHAVE_LLTD
endif

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
endif

ifeq ($(CONFIG_SER),y)
CFLAGS += -DHAVE_SER
endif
ifeq ($(CONFIG_HOTPLUG2),y)
CFLAGS += -DHAVE_HOTPLUG2
endif
ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS += -DHAVE_WZRHPAG300NH
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

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif
ifeq ($(CONFIG_GPIOWATCHER),y)
CFLAGS += -DHAVE_GPIOWATCHER
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DHAVE_FREEBIRD
endif
ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
endif
ifeq ($(CONFIG_UNBOUND),y)
CFLAGS += -DHAVE_UNBOUND
endif
ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFORWARD
endif

ifeq ($(CONFIG_DHCPRELAY),y)
CFLAGS += -DHAVE_DHCPRELAY
endif

ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -DHAVE_HTTPS
endif

ifeq ($(CONFIG_MATRIXSSL),y)
CFLAGS += -DHAVE_HTTPS
endif

ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
endif

ifeq ($(CONFIG_TMK),y)
CFLAGS += -DHAVE_TMK
endif
ifeq ($(CONFIG_IPVS),y)
CFLAGS += -DHAVE_IPVS
endif
ifeq ($(CONFIG_BKM),y)
CFLAGS += -DHAVE_BKM
endif
ifeq ($(CONFIG_FREERADIUS),y)
CFLAGS += -DHAVE_FREERADIUS
endif

#ifeq ($(CONFIG_MILKFISH),y)
#CFLAGS += -DHAVE_MILKFISH
#endif
ifeq ($(CONFIG_CONNTRACK),y)
CFLAGS += -DHAVE_CONNTRACK
endif
ifeq ($(CONFIG_RT2880),y)
CFLAGS += -DHAVE_RT2880
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
endif

ifeq ($(CONFIG_NVRAM_60K),y)
CFLAGS += -DHAVE_NVRAM_60K
endif
ifeq ($(CONFIG_NVRAM_64K),y)
CFLAGS += -DHAVE_NVRAM_64K
endif

ifeq ($(CONFIG_ATH9K),y)
LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
CFLAGS += -DHAVE_ATH9K
endif
ifeq ($(CONFIG_ATH10K),y)
CFLAGS += -DHAVE_ATH10K
endif

ifeq ($(CONFIG_CHRONY),y)
CFLAGS += -DHAVE_CHRONY
endif

ifeq ($(CONFIG_ANTAIRA),y)
CFLAGS += -DHAVE_ANTAIRA
ifeq ($(CONFIG_ANTAIRA_AGENT),y)
CFLAGS += -DHAVE_ANTAIRA_AGENT
endif
endif

include $(TOP)/rules/generic.mk

CFLAGS += $(DD_CFLAGS)

vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

%.o : %.c
	@echo "[CC] $@"
	$(CC) $(CFLAGS) -c -o $@ $<

all: rc init resetbutton wlanled wland et writetool ttraff process_monitor redial radio_timer watchdog hotplug servicemanager

clean:
	rm -f *.o rc init resetbutton wland et writetool
	rm -f *.c~

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/sbin
	install rc $(INSTALLDIR)/sbin	
ifneq ($(CONFIG_RT2880),y)
	install wland $(INSTALLDIR)/sbin	
endif
	install init $(INSTALLDIR)/sbin	
	install resetbutton $(INSTALLDIR)/sbin	

ifeq ($(CONFIG_UBNTM),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_WA850RE),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_PERU),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_ALFANX),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_CPE880),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_HABANERO),y)
	install wlanled $(INSTALLDIR)/sbin
endif
ifeq ($(CONFIG_CARAMBOLA),y)
ifeq ($(CONFIG_FMS2111),y)
	install wlanled $(INSTALLDIR)/sbin
endif
endif
ifeq ($(CONFIG_CPE890),y)
	install wlanled $(INSTALLDIR)/sbin
endif
	install ttraff $(INSTALLDIR)/sbin
	install process_monitor $(INSTALLDIR)/sbin
	install redial $(INSTALLDIR)/sbin
	install radio_timer $(INSTALLDIR)/sbin
	install hotplug $(INSTALLDIR)/sbin
	install servicemanager $(INSTALLDIR)/sbin
ifneq ($(CONFIG_MICRO),y)
ifneq ($(CONFIG_DIST),"micro")
	-install watchdog $(INSTALLDIR)/sbin
endif
endif
ifeq ($(CONFIG_ADM5120),y)
	install watchdog $(INSTALLDIR)/sbin
endif

ifeq ($(CONFIG_QTN),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc qtn_monitor
endif
ifeq ($(CONFIG_VENTANA),y)
	install update-prepare.sh $(INSTALLDIR)/usr/sbin	
	install update-after-pivot.sh $(INSTALLDIR)/usr/sbin	
	install ldd.sh $(INSTALLDIR)/usr/sbin
endif
ifeq ($(CONFIG_NEWPORT),y)
	install update-prepare.sh $(INSTALLDIR)/usr/sbin	
	install update-after-pivot.sh $(INSTALLDIR)/usr/sbin
	install ldd.sh $(INSTALLDIR)/usr/sbin
	install writetool $(INSTALLDIR)/usr/sbin
endif
ifeq ($(CONFIG_OCTEON),y)
	install update-prepare.sh $(INSTALLDIR)/usr/sbin	
	install update-after-pivot.sh $(INSTALLDIR)/usr/sbin	
	install ldd.sh $(INSTALLDIR)/usr/sbin
	install writetool $(INSTALLDIR)/usr/sbin
endif
ifeq ($(CONFIG_X86),y)
	install update-prepare.sh $(INSTALLDIR)/usr/sbin	
	install update-after-pivot.sh $(INSTALLDIR)/usr/sbin	
	install ldd.sh $(INSTALLDIR)/usr/sbin
	install writetool $(INSTALLDIR)/usr/sbin
endif
ifeq ($(CONFIG_RB600),y)
	install update-prepare.sh $(INSTALLDIR)/usr/sbin	
	install update-after-pivot.sh $(INSTALLDIR)/usr/sbin	
	install ldd.sh $(INSTALLDIR)/usr/sbin
	install writetool $(INSTALLDIR)/usr/sbin
endif
ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
	cd $(INSTALLDIR)/sbin && ln -sf rc brctl
endif
endif
ifeq ($(CONFIG_DIST),"micro-special")
	cd $(INSTALLDIR)/sbin && ln -sf rc brctl
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc erase
	cd $(INSTALLDIR)/sbin && ln -sf rc roaming_daemon
	cd $(INSTALLDIR)/sbin && ln -sf rc get_wanface
	cd $(INSTALLDIR)/sbin && ln -sf rc get_wanip
	cd $(INSTALLDIR)/sbin && ln -sf rc ledtool
	cd $(INSTALLDIR)/sbin && ln -sf rc write
	cd $(INSTALLDIR)/sbin && ln -sf rc restore
	cd $(INSTALLDIR)/sbin && ln -sf rc event
	cd $(INSTALLDIR)/sbin && ln -sf rc filter
#	cd $(INSTALLDIR)/sbin && ln -sf rc resetbutton
	cd $(INSTALLDIR)/sbin && ln -sf rc filtersync
	cd $(INSTALLDIR)/sbin && ln -sf rc ntpd
	cd $(INSTALLDIR)/sbin && ln -sf rc getbridge
	cd $(INSTALLDIR)/sbin && ln -sf rc getmask
	cd $(INSTALLDIR)/sbin && ln -sf rc getipmask
	cd $(INSTALLDIR)/sbin && ln -sf rc setportprio
	cd $(INSTALLDIR)/sbin && ln -sf rc ipupdated
#	cd $(INSTALLDIR)/sbin && ln -sf rc redial
	cd $(INSTALLDIR)/sbin && ln -sf rc setuserpasswd
	cd $(INSTALLDIR)/usr/sbin && ln -sf /sbin/rc nvram
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_connect
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_disconnect
	cd $(INSTALLDIR)/sbin && ln -sf rc gpio
	cd $(INSTALLDIR)/sbin && ln -sf rc softwarerevision
	cd $(INSTALLDIR)/sbin && ln -sf rc mtd
ifeq ($(CONFIG_GPIOWATCHER),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc gpiowatcher
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc beep
	cd $(INSTALLDIR)/sbin && ln -sf rc ledtracking
ifeq ($(CONFIG_IPV6),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc dhcp6c-state
endif
ifeq ($(CONFIG_WPS),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc wpswatcher
endif
ifeq ($(CONFIG_REGISTER),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc regshell
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc listen
	cd $(INSTALLDIR)/sbin && ln -sf rc supplicant
	cd $(INSTALLDIR)/sbin && ln -sf rc check_ps
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_success
#	cd $(INSTALLDIR)/sbin && ln -sf rc process_monitor
	cd $(INSTALLDIR)/sbin && ln -sf rc site_survey
#	cd $(INSTALLDIR)/sbin && ln -sf rc radio_timer
#	cd $(INSTALLDIR)/sbin && ln -sf rc ttraff	
ifeq ($(CONFIG_WIVIZ),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc autokill_wiviz
endif
ifeq ($(CONFIG_ARP),y)
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox arp
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc sendudp
	cd $(INSTALLDIR)/sbin && ln -sf rc misc	
	cd $(INSTALLDIR)/sbin && ln -sf rc gratarp
	cd $(INSTALLDIR)/sbin && ln -sf rc check_ses_led	
	cd $(INSTALLDIR)/sbin && ln -sf rc setpasswd
#	cd $(INSTALLDIR)/sbin && ln -sf rc wland
ifeq ($(CONFIG_NORTHSTAR),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc rtkswitch
endif
ifeq ($(CONFIG_PPPOESERVER),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc addpppoeconnected
	cd $(INSTALLDIR)/sbin && ln -sf rc delpppoeconnected
	cd $(INSTALLDIR)/sbin && ln -sf rc addpppoetime
endif
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager startservice
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager startservice_f
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager stopservice
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager stopservice_f
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager startservices
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager start_single_service
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager stopservices
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager restart
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager restart_f
	cd $(INSTALLDIR)/sbin && ln -sf servicemanager stop_running
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc get_nfmark
ifeq ($(CONFIG_PPTPD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc poptop
endif

ifeq ($(CONFIG_WOL),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc wol
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc ipfmt

wland: wland.o
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lwireless -lshutils -lqos


resetbutton: resetbutton.o
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lshutils

wlanled: wlanled.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ -L$(TOP)/libnl-tiny -lnl-tiny

ttraff: ttraff.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lshutils

process_monitor: process_monitor.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ -L$(TOP)/libutils -L$(TOP)/nvram -lshutils -lnvram -lutils

redial: redial.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ -L$(TOP)/libutils -L$(TOP)/nvram -lutils -lshutils -lnvram

servicemanager: servicemanager.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lshutils -lwireless

hotplug: hotplug.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ -L$(TOP)/libutils -L$(TOP)/nvram -lutils -lshutils -lnvram

watchdog: watchdog.o
ifneq ($(CONFIG_MICRO),y)
ifneq ($(CONFIG_DIST),"micro")
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lshutils -lwireless
else
	@true
endif
else
	@true
endif

radio_timer: radio_timer.o
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lshutils -lwireless

init:
	@echo "[LD] init"
	@$(CC) $(CFLAGS) $(LTO) $(LDLTO) $(INITCFLAGS) -L$(TOP)/libutils -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -o init init.c -lnvram -lshutils 

writetool:
	@echo "[LD] writetool"
	@$(CC) $(CFLAGS) $(LTO) $(LDLTO) $(INITCFLAGS) -o writetool writetool.c

et:
ifeq ($(CONFIG_NORTHSTAR),y)
	@echo "[LD] et"
	@$(CC) -Dlinux -I$(LINUXDIR)/brcm/arm/include -I$(LINUXDIR)/brcm/arm/common/include $(CFLAGS) -o et et.c
endif
	@true

rc: $(OBJS)
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $(LDLTO) -o $@ $^ $(LDFLAGS) -lwireless -lshutils -lqos

$(OBJS): $(CY_DEPS)
