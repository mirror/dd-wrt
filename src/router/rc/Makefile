include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
else
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
endif
else
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN -I$(LINUXDIR)/brcm/shared/bcmwifi/include -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include 
CFLAGS  += $(COPTS)
ifeq ($(ARCH),i386)
LDFLAGS	+= -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/libutils -lutils -ldl
CFLAGS += -I$(LINUXDIR)
else
LDFLAGS	+= -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/libutils -lutils -ldl
CFLAGS += -I$(LINUXDIR) -I$(TOP)/libutils -fPIC 

#ifneq ($(ARCHITECTURE),lsx)
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
#endif
#ifeq ($(ARCHITECTURE),lsx)
#CFLAGS += -fomit-frame-pointer 
#endif
endif
CFLAGS += $(MIPS16_OPT) 

cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)



INITCFLAGS = $(call cc-option,-Os -ffunction-sections -fdata-sections,)
# -fno-guess-branch-probability: prohibit pseudo-random guessing
# of branch probabilities (hopefully makes bloatcheck more stable):
#INITCFLAGS += $(call cc-option,-fno-guess-branch-probability,)
#INITCFLAGS += $(call cc-option,-funsigned-char -static-libgcc,)
#INITCFLAGS += $(call cc-option,-falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1,)

#LDFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections

OBJS := rc.o servicemanager.o event.o
#http.o stats.o
OBJS += watchdog.o services.o mtd_main.o

#ifdef $(CONFIG_DIST)
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"
#endif

CFLAGS += -DARCHITECTURE="\"$(ARCHITECTURE)\""

ifneq ($(CONFIG_RB500),y)
ifneq ($(CONFIG_X86),y)
OBJS += mtd.o
endif
endif
ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
endif
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
endif
ifeq ($(CONFIG_DNSMASQ),y)
CFLAGS += -DHAVE_DNSMASQ
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
ifeq ($(CONFIG_OVERCLOCKING),y)
CFLAGS += -DHAVE_OVERCLOCKING
endif
ifeq ($(CONFIG_80211AC),y)
  CFLAGS += -DHAVE_80211AC
endif
ifeq ($(CONFIG_NORTHSTAR),y)
  CFLAGS += -DHAVE_NORTHSTAR
endif
ifeq ($(CONFIG_ACK),y)
CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS += -DHAVE_JFFS2
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
ifeq ($(CONFIG_SOLO51),y)
CFLAGS += -DHAVE_SOLO51
endif
ifeq ($(CONFIG_KAID),y)
CFLAGS += -DHAVE_KAID
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
endif
ifeq ($(CONFIG_WASP),y)
CFLAGS += -DHAVE_WASP
endif
ifeq ($(CONFIG_DIR825C1),y)
CFLAGS += -DHAVE_DIR825C1
endif
ifeq ($(CONFIG_RB2011),y)
CFLAGS += -DHAVE_RB2011
endif
ifeq ($(CONFIG_WDR4300),y)
CFLAGS += -DHAVE_WDR4300
endif
ifeq ($(CONFIG_WHR300HP2),y)
CFLAGS += -DHAVE_WHR300HP2
endif
ifeq ($(CONFIG_WDR4900),y)
CFLAGS += -DHAVE_WDR4900
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
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
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
ifeq ($(CONFIG_VENTANA),y)
  CFLAGS += -DHAVE_VENTANA
endif
ifeq ($(CONFIG_NEPTUNE),y)
CFLAGS += -DHAVE_NEPTUNE
endif
ifeq ($(CONFIG_RT3352),y)
CFLAGS += -DHAVE_RT3352
endif
ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
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
ifeq ($(CONFIG_WMBR_G300NH),y)
CFLAGS += -DHAVE_WMBR_G300NH
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
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
ifeq ($(CONFIG_IPR),y)
CFLAGS += -DHAVE_IPR
endif
ifeq ($(CONFIG_ONNET),y)
CFLAGS += -DHAVE_ONNET
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
ifeq ($(CONFIG_RADIOOFF),y)
CFLAGS += -DHAVE_RADIOOFF
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
ifeq ($(CONFIG_JWAP003),y)
CFLAGS += -DHAVE_JWAP003
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
ifeq ($(CONFIG_WR703),y)
CFLAGS += -DHAVE_WR703
endif
ifeq ($(CONFIG_MR3020),y)
CFLAGS += -DHAVE_MR3020
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
ifeq ($(CONFIG_DIR615I),y)
CFLAGS += -DHAVE_DIR615I
endif
ifeq ($(CONFIG_DIR615),y)
CFLAGS += -DHAVE_DIR615
endif
ifeq ($(CONFIG_TEW632BRP),y)
CFLAGS += -DHAVE_TEW632BRP
endif
ifeq ($(CONFIG_UBNTM),y)
CFLAGS += -DHAVE_UBNTM
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
OBJS += run_wiviz.o
OBJS += autokill_wiviz.o
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

ifeq ($(CONFIG_FTP),y)
CFLAGS += -DHAVE_FTP
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_TIEXTRA1),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_TIEXTRA2),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_SAMBA3),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
CFLAGS += -DHAVE_SAMBA3
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

CFLAGS += -DHAVE_AQOS

ifeq ($(CONFIG_PPTPD),y)
CFLAGS += -DHAVE_PPTPD
OBJS += pptpd.o
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
ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
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

ifeq ($(CONFIG_RSTP),y)
CFLAGS += -DHAVE_RSTP
endif

ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DHAVE_BCMMODERN
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS +=  -DHAVE_MADWIFI -DHAVE_ROAMING 
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS +=  -DHAVE_MADWIFI_MIMO
endif



ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
endif

ifeq ($(CONFIG_WOL),y)
OBJS +=wol.o
CFLAGS += -DHAVE_WOL
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS += -DHAVE_NOCAT
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

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DHAVE_FREEBIRD
endif
ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
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
ifeq ($(CONFIG_BKM),y)
CFLAGS += -DHAVE_BKM
endif
ifeq ($(CONFIG_FREERADIUS),y)
CFLAGS += -DHAVE_FREERADIUS
endif

ifeq ($(CONFIG_MILKFISH),y)
CFLAGS += -DHAVE_MILKFISH
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

include $(TOP)/rules/generic.mk
CFLAGS += $(DD_CFLAGS)

vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: rc

clean:
	rm -f *.o rc
	rm -f *.c~

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/sbin
	install rc $(INSTALLDIR)/sbin	
	install ttraff $(INSTALLDIR)/sbin	
ifneq ($(CONFIG_RT2880),y)
	install wland $(INSTALLDIR)/sbin	
endif
	install process_monitor $(INSTALLDIR)/sbin	
	install resetbutton $(INSTALLDIR)/sbin	
ifeq ($(CONFIG_GPIOWATCHER),y)
	install gpiowatcher $(INSTALLDIR)/sbin	
endif
	install init $(INSTALLDIR)/sbin	
	install check_ps $(INSTALLDIR)/sbin	
	install nvram $(INSTALLDIR)/usr/sbin	
	install ledtool $(INSTALLDIR)/sbin	
ifeq ($(CONFIG_WPS),y)
	install wpswatcher $(INSTALLDIR)/sbin	
endif
ifeq ($(CONFIG_UBNTM),y)
	install wlanled $(INSTALLDIR)/sbin	
endif
#	install event $(INSTALLDIR)/sbin	
	install radio_timer $(INSTALLDIR)/sbin
	install hotplug $(INSTALLDIR)/sbin
	install listen $(INSTALLDIR)/sbin
	$(STRIP) $(INSTALLDIR)/sbin/rc
#	cd $(INSTALLDIR)/sbin && ln -sf rc init
ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
	cd $(INSTALLDIR)/sbin && ln -sf rc brctl
endif
endif
ifeq ($(CONFIG_DIST),"micro-special")
	cd $(INSTALLDIR)/sbin && ln -sf rc brctl
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc erase
#	cd $(INSTALLDIR)/sbin && ln -sf rc roaming_daemon
	cd $(INSTALLDIR)/sbin && ln -sf rc get_wanface
#	cd $(INSTALLDIR)/sbin && ln -sf rc ledtool
	cd $(INSTALLDIR)/sbin && ln -sf rc write
	cd $(INSTALLDIR)/sbin && ln -sf rc restore
#	cd $(INSTALLDIR)/sbin && ln -sf rc stats
	cd $(INSTALLDIR)/sbin && ln -sf rc event
#	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug  //done in hotplug.c
	cd $(INSTALLDIR)/sbin && ln -sf rc filter
#	cd $(INSTALLDIR)/sbin && ln -sf rc resetbutton
	cd $(INSTALLDIR)/sbin && ln -sf rc filtersync
	cd $(INSTALLDIR)/sbin && ln -sf rc ntpd
	cd $(INSTALLDIR)/sbin && ln -sf rc getbridge
	cd $(INSTALLDIR)/sbin && ln -sf rc getmask
	cd $(INSTALLDIR)/sbin && ln -sf rc getbridgeprio
	cd $(INSTALLDIR)/sbin && ln -sf rc ipupdated
	cd $(INSTALLDIR)/sbin && ln -sf rc redial
	cd $(INSTALLDIR)/sbin && ln -sf rc setuserpasswd
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /sbin/rc nvram
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_connect
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_disconnect
	cd $(INSTALLDIR)/sbin && ln -sf rc gpio
	cd $(INSTALLDIR)/sbin && ln -sf rc softwarerevision
	cd $(INSTALLDIR)/sbin && ln -sf rc mtd
	cd $(INSTALLDIR)/sbin && ln -sf rc beep
	cd $(INSTALLDIR)/sbin && ln -sf rc ledtracking
ifeq ($(CONFIG_REGISTER),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc regshell
endif
#	cd $(INSTALLDIR)/sbin && ln -sf rc listen
	cd $(INSTALLDIR)/sbin && ln -sf rc supplicant
#	cd $(INSTALLDIR)/sbin && ln -sf rc check_ps
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_success
#	cd $(INSTALLDIR)/sbin && ln -sf rc process_monitor
	cd $(INSTALLDIR)/sbin && ln -sf rc site_survey
#	cd $(INSTALLDIR)/sbin && ln -sf rc radio_timer
#	cd $(INSTALLDIR)/sbin && ln -sf rc ttraff	
ifeq ($(CONFIG_WIVIZ),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_wiviz
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
ifneq ($(CONFIG_MICRO),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
endif
ifeq ($(CONFIG_ADM5120),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
endif
ifeq ($(CONFIG_PPPOESERVER),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc addpppoeconnected
	cd $(INSTALLDIR)/sbin && ln -sf rc delpppoeconnected
	cd $(INSTALLDIR)/sbin && ln -sf rc addpppoetime
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc startservice
	cd $(INSTALLDIR)/sbin && ln -sf rc startservice_f
#	cd $(INSTALLDIR)/sbin && ln -sf rc switch
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservice
	cd $(INSTALLDIR)/sbin && ln -sf rc startservices
	cd $(INSTALLDIR)/sbin && ln -sf rc start_single_service
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservices
	cd $(INSTALLDIR)/sbin && ln -sf rc startstop
	cd $(INSTALLDIR)/sbin && ln -sf rc startstop_f
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc get_nfmark
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox httpd
#ifeq ($(CONFIG_BIRD),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox bird
#endif

#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dnsmasq
	cd $(INSTALLDIR)/usr/sbin && ln -sf /usr/sbin/iptables iptables-restore
#ifeq ($(CONFIG_UPNP),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox upnp
#endif
#ifeq ($(CONFIG_DDNS),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox ez-ipupdate
#endif
#ifeq ($(CONFIG_PPPD),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox pppd
#endif
ifeq ($(CONFIG_PPTPD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc poptop
endif

ifeq ($(CONFIG_WOL),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc wol
endif

#ifeq ($(CONFIG_DDLAN),y)
#	cp check.sh $(INSTALLDIR)/sbin
#endif



rc: $(OBJS)
#	$(AR) arc -o $@.a $^
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lwireless
ifeq ($(CONFIG_MADWIFI),y)
	$(CC) $(CFLAGS) -o roaming_daemon roaming_daemon.c $(LDFLAGS) ../wireless-tools/libiw.so 
endif
	$(CC) $(CFLAGS) -o ttraff ttraff.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o wland wland.c $(LDFLAGS) -lwireless
	$(CC) $(CFLAGS) -o process_monitor process_monitor.c ntp.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o resetbutton resetbutton.c $(LDFLAGS) -lwireless
	$(CC) $(CFLAGS) -o gpiowatcher gpiowatcher.c $(LDFLAGS)
#ifeq ($(CONFIG_LSX),y)
#	$(CC) $(CFLAGS) $(INITCFLAGS) -L$(TOP)/libutils -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -o init init.c $(TOP)/nvram/libnvram.a $(TOP)/libutils/libutils.a
#else
	$(CC) $(CFLAGS) $(INITCFLAGS) -L$(TOP)/libutils -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -o init init.c -lnvram -lutils  $(LDFLAGS)
#endif
	$(CC) $(CFLAGS) -o check_ps check_ps.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o nvram nvram.c -L../nvram -lnvram  -L$(TOP)/libutils -lutils $(LDFLAGS)
	$(CC) $(CFLAGS) -o ledtool ledtool.c $(LDFLAGS)
ifeq ($(CONFIG_WPS),y)
	$(CC) $(CFLAGS) -o wpswatcher wpswatcher.c $(LDFLAGS)
endif
ifeq ($(CONFIG_UBNTM),y)
	$(CC) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include $(CFLAGS)  -I$(TOP)/libubox -I$(TOP)/libnl-tiny/include -o wlanled wlanled.c $(LDFLAGS) -lwireless
endif
#	$(CC) $(CFLAGS) -o event event.c
	$(CC) $(CFLAGS) -o radio_timer radio_timer.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o hotplug hotplug.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o listen listen.c $(LDFLAGS)

$(OBJS): $(CY_DEPS)
