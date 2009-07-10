

include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

LDFLAGS	= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lcrypt -L../libnet/lib -lnet -L../libutils -lutils
LDFLAGS += -lm

CFLAGS	= $(COPTS) -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS  += -fpic -I$(TOP)/services/include


HCFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
HCFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
HCFLAGS  += -fpic -I$(TOP)/services/include


OBJS = services.o network.o firewall.o mssid.o wshaper.o ddns.o sysinit.o sysinit-$(ARCHITECTURE).o interface.o udhcpc.o mkfiles.o iptable.o defaults.o \
       bridgetools.o igmp.o wpa.o vlantagging.o httpd.o setuserpw.o anchorfree.o gpio.o mask.o reset_defaults.o


ifeq ($(CONFIG_WHRAG108),y)
else
ifeq ($(CONFIG_TW6600),y)
else
ifeq ($(CONFIG_FONERA),y)
OBJS += overclock_atheros.o
else
ifeq ($(CONFIG_DIR300),y)
OBJS += overclock_atheros.o
else
ifeq ($(CONFIG_LS2),y)
OBJS += overclock_atheros.o
else
ifeq ($(CONFIG_LS5),y)
OBJS += overclock_atheros.o
else
ifeq ($(CONFIG_CA8),y)
# warning. support must be implemented for zLoader bootloaders. in progress
OBJS += overclock_atheros.o 
else
ifeq ($(CONFIG_MERAKI),y)
OBJS += overclock_atheros.o
else
ifneq ($(CONFIG_XSCALE),y)
endif
endif
endif
endif
endif
endif
endif
endif
endif
ifeq ($(CONFIG_RSTATS),y)
CFLAGS_EXTRA += -DHAVE_RSTATS
OBJS += rstats.o
endif
ifeq ($(CONFIG_ALLNETWRT),y)
  CFLAGS_EXTRA += -DHAVE_ALLNETWRT
endif
ifeq ($(CONFIG_OVERCLOCKING),y)
CFLAGS_EXTRA += -DHAVE_OVERCLOCKING
endif
ifeq ($(CONFIG_IPV6),y)
CFLAGS_EXTRA += -DHAVE_IPV6
endif
CFLAGS_EXTRA += -DARCH_$(ARCHITECTURE)
ifeq ($(CONFIG_TRIMAX),y)
CFLAGS_EXTRA += -DHAVE_TRIMAX
endif
ifeq ($(CONFIG_EAD),y)
CFLAGS_EXTRA += -DHAVE_EAD
endif
ifeq ($(CONFIG_ACK),y)
CFLAGS_EXTRA += -DHAVE_ACK
endif
ifeq ($(CONFIG_SWCONFIG),y)
CFLAGS_EXTRA += -DHAVE_SWCONFIG
endif
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
  CFLAGS_EXTRA += -DBUFFALO_JP
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS_EXTRA += -DHAVE_MADWIFI_MIMO
endif
ifeq ($(CONFIG_GLAUCO),y)
CFLAGS_EXTRA += -DHAVE_GLAUCO
endif

ifeq ($(CONFIG_NSTX),y)
CFLAGS_EXTRA += -DHAVE_NSTX
OBJS += nstx.o
endif
ifeq ($(CONFIG_UDHCPD),y)
CFLAGS_EXTRA += -DHAVE_UDHCPD
OBJS += udhcpd.o
endif

ifeq ($(CONFIG_WPA_SUPPLICANT),y)
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
CFLAGS_EXTRA += -DHAVE_WPA_SUPPLICANT
OBJS += brcm_80211x.o
endif
endif
endif
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
CFLAGS_EXTRA += -DHAVE_WPA_SUPPLICANT
OBJS += brcm_80211x.o
endif
endif
endif
ifeq ($(CONFIG_DNSMASQ),y)
CFLAGS_EXTRA += -DHAVE_DNSMASQ
OBJS += dnsmasq.o
endif
ifeq ($(CONFIG_KAID),y)
CFLAGS_EXTRA += -DHAVE_KAID
endif
ifeq ($(CONFIG_OLED),y)
CFLAGS_EXTRA += -DHAVE_OLED
endif
ifeq ($(CONFIG_TONZE),y)
CFLAGS_EXTRA += -DHAVE_TONZE
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS_EXTRA += -DHAVE_SYSLOG
endif
ifeq ($(CONFIG_SPUTNIK),y)
CFLAGS_EXTRA += -DHAVE_SPUTNIK
endif
ifeq ($(CONFIG_MMC),y)
CFLAGS_EXTRA += -DHAVE_MMC
OBJS += mmc.o
endif
ifeq ($(CONFIG_PPPOESERVER),y)
CFLAGS_EXTRA += -DHAVE_PPPOESERVER
OBJS += pppoeserver.o
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS_EXTRA += -DHAVE_JFFS2
OBJS += jffs.o
endif
ifeq ($(CONFIG_WGT624),y)
  CFLAGS_EXTRA += -DHAVE_WGT624
endif
ifeq ($(CONFIG_GWMF54G2),y)
  CFLAGS_EXTRA += -DHAVE_GWMF54G2
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS_EXTRA += -DHAVE_BUFFALO
endif
ifeq ($(CONFIG_3G),y)
CFLAGS_EXTRA += -DHAVE_3G
endif

ifeq ($(CONFIG_USB),y)
CFLAGS_EXTRA += -DHAVE_USB
OBJS += usb_hotplug.o
endif
ifeq ($(CONFIG_ALFA_BRANDING1),y)
CFLAGS_EXTRA += -DHAVE_ALFA_BRANDING
endif
ifeq ($(CONFIG_ALFA_BRANDING2),y)
CFLAGS_EXTRA += -DHAVE_ALFA_BRANDING
endif
ifeq ($(CONFIG_ALPHA),y)
CFLAGS_EXTRA += -DHAVE_ALPHA
endif
ifeq ($(CONFIG_CORENET),y)
CFLAGS_EXTRA += -DHAVE_CORENET
endif
ifeq ($(CONFIG_XIOCOM),y)
CFLAGS_EXTRA += -DHAVE_XIOCOM
endif

ifeq ($(CONFIG_USB_ADVANCED),y)
CFLAGS_EXTRA += -DHAVE_USB_ADVANCED
endif
ifeq ($(CONFIG_WRK54G),y)
CFLAGS_EXTRA += -DHAVE_WRK54G
endif
ifeq ($(CONFIG_RSTP),y)
CFLAGS_EXTRA += -DHAVE_RSTP
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS_EXTRA += -DHAVE_VLANTAGGING
endif
ifeq ($(CONFIG_GW700),y)
CFLAGS_EXTRA += -DHAVE_GW700
endif
ifeq ($(CONFIG_WIVIZ),y)
CFLAGS_EXTRA += -DHAVE_WIVIZ
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS_EXTRA += -DHAVE_REGISTER
endif
ifeq ($(CONFIG_BRANDING),y)
CFLAGS_EXTRA += -DCONFIG_BRANDING
endif
ifeq ($(CONFIG_THOM),y)
CFLAGS_EXTRA += -DHAVE_THOM
endif
ifeq ($(CONFIG_MADWIFI),y)
OBJS += site_survey_madwifi.o
OBJS += stabridge.o beep.o
else
ifeq ($(CONFIG_RT2880),y)
OBJS += site_survey_ralink.o
else
OBJS += site_survey_broadcom.o
endif
endif

ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS_EXTRA += -fpic
endif

ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS_EXTRA += -DHAVE_ONLYCLIENT
endif

#ifdef $(CONFIG_DIST)
CFLAGS_EXTRA += -DDIST=\"$(CONFIG_DIST)\"
#endif
ifeq ($(CONFIG_34TELECOM),y)
CFLAGS_EXTRA += -DHAVE_34TELECOM
endif
ifeq ($(CONFIG_SAMBA),y)
CFLAGS_EXTRA += -DHAVE_SAMBA
endif
ifeq ($(CONFIG_WIFIDOG),y)
CFLAGS_EXTRA += -DHAVE_WIFIDOG
OBJS += wifidog.o
endif
ifeq ($(CONFIG_BONDING),y)
CFLAGS_EXTRA += -DHAVE_BONDING
OBJS += bonding.o
endif
ifeq ($(CONFIG_RADLOCAL),y)
CFLAGS_EXTRA += -DHAVE_RADLOCAL
endif
ifeq ($(CONFIG_GEMTEK),y)
CFLAGS_EXTRA += -DHAVE_GEMTEK
endif

ifeq ($(CONFIG_KODATA),y)
CFLAGS_EXTRA += -DHAVE_KODATA
endif

ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS_EXTRA += -DHAVE_MICRO
OBJS += libbridge_if.o
OBJS += libbridge_init.o
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS_EXTRA += -DHAVE_MICRO
OBJS += libbridge_if.o
OBJS += libbridge_init.o
endif
endif

ifeq ($(CONFIG_RADIOOFF),y)
  CFLAGS_EXTRA += -DHAVE_RADIOOFF
endif
ifeq ($(CONFIG_MEDIASERVER),y)
CFLAGS_EXTRA += -DHAVE_MEDIASERVER
endif
ifeq ($(CONFIG_L2TP),y)
CFLAGS_EXTRA += -DHAVE_L2TP
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS_EXTRA += -DHAVE_NOWIFI
endif
ifeq ($(CONFIG_PPPOE),y)
CFLAGS_EXTRA += -DHAVE_PPPOE
OBJS += ppp.o
endif
ifeq ($(CONFIG_PPPOERELAY),y)
CFLAGS_EXTRA += -DHAVE_PPPOERELAY
OBJS += pppoerelay.o
endif

ifeq ($(CONFIG_DDLAN),y)
CFLAGS_EXTRA += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS_EXTRA += -DHAVE_HEARTBEAT
OBJS += heartbeat.o
endif
ifeq ($(CONFIG_ECB9750),y)
  CFLAGS_EXTRA += -DHAVE_ECB9750
endif

ifeq ($(CONFIG_RT2880),y)
OBJS += rt2880.o
endif

ifeq ($(CONFIG_WG302),y)
  CFLAGS_EXTRA += -DHAVE_WG302
endif
ifeq ($(CONFIG_WG302V1),y)
  CFLAGS_EXTRA += -DHAVE_WG302V1
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS_EXTRA += -DHAVE_MADWIFI -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL

ifneq ($(CONFIG_NOWIFI),y)
OBJS += madwifi.o
OBJS += ../wireless-tools/libiw.so.29
endif
endif

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS_EXTRA += -DHAVE_POWERNOC
endif
ifeq ($(CONFIG_BOESE),y)
CFLAGS_EXTRA += -DBOESE=1
endif
ifeq ($(CONFIG_WILLIAM),y)
CFLAGS_EXTRA += -DWILLIAM=1
endif

ifeq ($(CONFIG_MULTICAST),y)
CFLAGS_EXTRA += -DHAVE_MULTICAST
endif

ifeq ($(CONFIG_FON),y)
CFLAGS_EXTRA += -DHAVE_FON
endif
ifeq ($(CONFIG_UPNP),y)
CFLAGS_EXTRA += -DHAVE_UPNP
OBJS += upnp.o
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
CFLAGS_EXTRA += -DHAVE_CHILLILOCAL
OBJS += chillispot.o
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS_EXTRA += -DHAVE_TFTP
endif
ifeq ($(CONFIG_ERC),y)
CFLAGS_EXTRA += -DHAVE_ERC
endif

ifeq ($(CONFIG_FTP),y)
CFLAGS_EXTRA += -DHAVE_FTP
OBJS += proftp.o
endif
ifeq ($(CONFIG_SAMBA_SRV),y)
CFLAGS_EXTRA += -DHAVE_SAMBA_SRV
OBJS += sambasrv.o
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS_EXTRA += -DHAVE_RB500
endif
ifeq ($(CONFIG_XSCALE),y)
CFLAGS_EXTRA += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS_EXTRA += -DHAVE_PORTSETUP
endif
ifeq ($(CONFIG_CPUTEMP),y)
CFLAGS_EXTRA += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_WRT54G2),y)
CFLAGS_EXTRA += -DHAVE_WRT54G2
endif
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS_EXTRA += -DHAVE_MAGICBOX
CFLAGS_EXTRA += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_DIR400),y)
CFLAGS_EXTRA += -DHAVE_FONERA
CFLAGS_EXTRA += -DHAVE_DIR400
OBJS += icplus.o
else
ifeq ($(CONFIG_DIR300),y)
CFLAGS_EXTRA += -DHAVE_FONERA
CFLAGS_EXTRA += -DHAVE_DIR300
OBJS += icplus.o
endif
endif
ifeq ($(CONFIG_MR3202A),y)
CFLAGS_EXTRA += -DHAVE_MR3202A
OBJS += admtek.o
endif
ifeq ($(CONFIG_FONERA),y)
CFLAGS_EXTRA += -DHAVE_FONERA
endif
ifeq ($(CONFIG_MERAKI),y)
CFLAGS_EXTRA += -DHAVE_FONERA
CFLAGS_EXTRA += -DHAVE_MERAKI
endif
ifeq ($(CONFIG_EAP3660),y)
  CFLAGS_EXTRA += -DHAVE_EAP3660
endif
ifeq ($(CONFIG_ECB3500),y)
  CFLAGS_EXTRA += -DHAVE_ECB3500
endif
ifeq ($(CONFIG_EOC2610),y)
  CFLAGS_EXTRA += -DHAVE_EOC2610
endif
ifeq ($(CONFIG_EOC1650),y)
  CFLAGS_EXTRA += -DHAVE_EOC1650
endif
ifeq ($(CONFIG_BS2HP),y)
CFLAGS_EXTRA += -DHAVE_BS2HP
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_LC2),y)
CFLAGS_EXTRA += -DHAVE_LC2
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_BS2),y)
CFLAGS_EXTRA += -DHAVE_BS2
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_PICO2),y)
CFLAGS_EXTRA += -DHAVE_PICO2
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_PICO2HP),y)
CFLAGS_EXTRA += -DHAVE_PICO2HP
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_MS2),y)
CFLAGS_EXTRA += -DHAVE_MS2
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_NS2),y)
CFLAGS_EXTRA += -DHAVE_NS2
CFLAGS_EXTRA += -DHAVE_LS2
else
ifeq ($(CONFIG_LS2),y)
CFLAGS_EXTRA += -DHAVE_LS2
OBJS += icplus.o
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(CONFIG_BWRG1000),y)
  CFLAGS_EXTRA += -DHAVE_BWRG1000
endif

ifeq ($(CONFIG_NS5),y)
CFLAGS_EXTRA += -DHAVE_NS5
endif
ifeq ($(CONFIG_EOC5610),y)
CFLAGS_EXTRA += -DHAVE_EOC5610
endif
ifeq ($(CONFIG_NS3),y)
CFLAGS_EXTRA += -DHAVE_NS3
endif
ifeq ($(CONFIG_LC5),y)
CFLAGS_EXTRA += -DHAVE_LC5
endif
ifeq ($(CONFIG_BS5),y)
CFLAGS_EXTRA += -DHAVE_BS5
endif
ifeq ($(CONFIG_PICO5),y)
CFLAGS_EXTRA += -DHAVE_PICO5
endif

ifeq ($(CONFIG_LS5),y)
CFLAGS_EXTRA += -DHAVE_LS5
endif
ifeq ($(CONFIG_WHRAG108),y)
CFLAGS_EXTRA += -DHAVE_WHRAG108
endif
ifeq ($(CONFIG_PB42),y)
CFLAGS_EXTRA += -DHAVE_PB42
endif
ifeq ($(CONFIG_LSX),y)
CFLAGS_EXTRA += -DHAVE_LSX
endif
ifeq ($(CONFIG_RS),y)
CFLAGS_EXTRA += -DHAVE_RS
OBJS += overclock_routerstation.o
endif
ifeq ($(CONFIG_DANUBE),y)
CFLAGS_EXTRA += -DHAVE_DANUBE
endif
ifeq ($(CONFIG_STORM),y)
CFLAGS_EXTRA += -DHAVE_STORM
endif
ifeq ($(CONFIG_ADM5120),y)
CFLAGS_EXTRA += -DHAVE_ADM5120
endif
ifeq ($(CONFIG_WP54G),y)
CFLAGS_EXTRA += -DHAVE_WP54G
endif
ifeq ($(CONFIG_NP28G),y)
CFLAGS_EXTRA += -DHAVE_NP28G
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS_EXTRA += -DHAVE_TW6600
endif
ifeq ($(CONFIG_CA8PRO),y)
CFLAGS_EXTRA += -DHAVE_CA8PRO
endif
ifeq ($(CONFIG_RCAA01),y)
CFLAGS_EXTRA += -DHAVE_RCAA01
endif
ifeq ($(CONFIG_RDAT81),y)
CFLAGS_EXTRA += -DHAVE_RDAT81
endif
ifeq ($(CONFIG_CA8),y)
CFLAGS_EXTRA += -DHAVE_CA8
endif
ifeq ($(CONFIG_WRT300NV2),y)
CFLAGS_EXTRA += -DHAVE_WRT300NV2
endif
ifeq ($(CONFIG_CAMBRIA),y)
CFLAGS_EXTRA += -DHAVE_CAMBRIA
endif
ifeq ($(CONFIG_VNCREPEATER),y)
CFLAGS_EXTRA += -DHAVE_VNCREPEATER
OBJS += vncrepeater.o
endif
ifeq ($(CONFIG_WAVESAT),y)
CFLAGS_EXTRA += -DHAVE_WAVESAT
OBJS += wavesat.o
endif
ifeq ($(CONFIG_NOP8670),y)
CFLAGS_EXTRA += -DHAVE_NOP8670
CFLAGS_EXTRA += -DHAVE_GATEWORX
else
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS_EXTRA += -DHAVE_GATEWORX
ifneq ($(CONFIG_WRT300NV2),y)
CFLAGS_EXTRA += -DHAVE_CPUTEMP
CFLAGS_EXTRA += -DHAVE_VOLT
endif
endif
endif
ifeq ($(CONFIG_X86),y)
CFLAGS_EXTRA += -DHAVE_X86
CFLAGS_EXTRA += -DHAVE_USB
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS_EXTRA += -DHAVE_OPENVPN
ifeq ($(CONFIG_AIRNET),y)
OBJS += openvpn-airnet.o
else
OBJS += openvpn.o
endif
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
CFLAGS_EXTRA += -DHAVE_EOP
endif
ifeq ($(CONFIG_IPROUTE2),y)
CFLAGS_EXTRA += -DHAVE_IPROUTE2
endif

ifeq ($(CONFIG_ROUTERSTYLE),y)
CFLAGS_EXTRA += -DHAVE_ROUTERSTYLE
endif

ifeq ($(CONFIG_TELCOM),y)
CFLAGS_EXTRA += -DHAVE_TELCOM
endif

ifeq ($(CONFIG_GGEW),y)
CFLAGS_EXTRA += -DHAVE_GGEW
CFLAGS_EXTRA += -DHAVE_NEWMEDIA
endif

ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS_EXTRA += -DHAVE_NEWMEDIA
endif
ifeq ($(CONFIG_SKYTRON),y)
CFLAGS_EXTRA += -DHAVE_SKYTRON
endif

ifeq ($(CONFIG_SKYTEL),y)
CFLAGS_EXTRA += -DHAVE_SKYTEL
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS_EXTRA += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_MAKSAT),y)
CFLAGS_EXTRA += -DHAVE_MAKSAT
endif

ifeq ($(CONFIG_MAKSAT_BLANK),y)
CFLAGS_EXTRA += -DHAVE_MAKSAT_BLANK
endif

ifeq ($(CONFIG_TMK),y)
CFLAGS_EXTRA += -DHAVE_TMK
endif

ifeq ($(CONFIG_VILIM),y)
CFLAGS_EXTRA += -DHAVE_VILIM
endif

ifeq ($(CONFIG_MAKSAT_BROADCOM),y)
CFLAGS_EXTRA += -DHAVE_MAKSAT_BROADCOM
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS_EXTRA += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_EBTABLES),y)
CFLAGS_EXTRA += -DHAVE_EBTABLES
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS_EXTRA += -DHAVE_OMNI
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS_EXTRA += -DHAVE_DLS
endif

ifeq ($(CONFIG_TELNET),y)
CFLAGS_EXTRA += -DHAVE_TELNET
OBJS += telnet.o
endif


ifeq ($(CONFIG_AQOS),y)
CFLAGS_EXTRA += -DHAVE_AQOS
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS_EXTRA += -DHAVE_BUFFALO -DDEFAULT_COUNTRY_CODE=\"$(CONFIG_DEFAULT_COUNTRYCODE)\" -DDEFAULT_LANGUAGE=\"$(CONFIG_DEFAULT_LANGUAGE)\"
endif

ifeq ($(CONFIG_PPTP),y)
CFLAGS_EXTRA += -DHAVE_PPTP
endif
ifeq ($(CONFIG_PPTPD),y)
CFLAGS_EXTRA += -DHAVE_PPTPD
OBJS += pptp.o
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS_EXTRA += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
OBJS += sshd.o
CFLAGS_EXTRA += -DHAVE_SSHD
endif

ifeq ($(CONFIG_RADVD),y)
CFLAGS_EXTRA += -DHAVE_RADVD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
OBJS += dhcpforward.o
CFLAGS_EXTRA += -DHAVE_DHCPFWD
endif
ifeq ($(CONFIG_PPPD),y)
CFLAGS_EXTRA += -DHAVE_PPPD
OBJS += ppp.o
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS_EXTRA += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS_EXTRA += -DHAVE_BIRD
OBJS += routing.o
endif

ifeq ($(CONFIG_PPP),y)
CFLAGS_EXTRA += -DHAVE_PPP
endif

ifeq ($(CONFIG_QUAGGA),y)
CFLAGS_EXTRA += -DHAVE_QUAGGA
OBJS += routing.o
endif

ifeq ($(CONFIG_WSHAPER),y)
OBJS += wshaper.o
CFLAGS_EXTRA += -DHAVE_WSHAPER
endif

ifeq ($(CONFIG_SVQOS),y)
CFLAGS_EXTRA += -DHAVE_SVQOS
endif

ifeq ($(CONFIG_SNMP),y)
CFLAGS_EXTRA += -DHAVE_SNMP
OBJS += snmp.o
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS_EXTRA += -DHAVE_WOL
OBJS += wol.o
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS_EXTRA += -DHAVE_NOCAT
OBJS += nocat.o
endif

ifeq ($(CONFIG_SER),y)
CFLAGS_EXTRA += -DHAVE_SER
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS_EXTRA += -DANTI_FLASH
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS_EXTRA += -DHAVE_FREEBIRD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS_EXTRA += -DHAVE_DHCPFORWARD
endif

ifeq ($(CONFIG_DHCPRELAY),y)

CFLAGS_EXTRA += -DHAVE_DHCPRELAY
endif

ifeq ($(CONFIG_OPENSSL),y)
CFLAGS_EXTRA += -DHAVE_HTTPS
endif

ifeq ($(CONFIG_MATRIXSSL),y)
CFLAGS_EXTRA += -DHAVE_HTTPS

endif
ifeq ($(CONFIG_OLSRD),y)
CFLAGS_EXTRA += -DHAVE_OLSRD
OBJS += olsrd.o
endif

ifeq ($(CONFIG_MILKFISH),y)
CFLAGS_EXTRA += -DHAVE_MILKFISH
OBJS += milkfish.o
endif

ifeq ($(CONFIG_ALLNET11N),y)
CFLAGS_EXTRA += -DHAVE_ALLNET11N
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS_EXTRA += -DHAVE_SPUTNIK_APD
OBJS += sputnik.o
endif

ifeq ($(CONFIG_RT2880),y)
  CFLAGS_EXTRA += -DHAVE_RT2880
endif
ifeq ($(CONFIG_WHRG300N),y)
  CFLAGS_EXTRA += -DHAVE_WHRG300N
endif
ifeq ($(CONFIG_ESR6650),y)
  CFLAGS_EXTRA += -DHAVE_ESR6650
endif
ifeq ($(CONFIG_NAS),y)
  CFLAGS_EXTRA += -DHAVE_NAS
endif

CFLAGS += $(CFLAGS_EXTRA)
HCFLAGS += $(CFLAGS_EXTRA)

vpath %.c sysinit sysinit/switchlib services tools networking $(TOP)/shared $(SRCBASE)/rts/src

all: services.so services.a
ifeq ($(CONFIG_RT2880),y)
	$(CC) $(CFLAGS) -DNEED_PRINTF -o switch tools/switch.c
endif
	gcc -o bin/defaults $(HCFLAGS) bin/defaults.c
	cd bin && ./defaults
    
clean:
	rm -f *.o *.a *.so
	rm -f *.c~ services/*.c~ networking/*.c~ tools/*.c~ sysinit/*.c~
	rm -f *.h~ services/*.h~ networking/*.h~ tools/*.h~ sysinit/*.h~

install: all
	install -d $(INSTALLDIR)/services/usr/lib	
	install -d $(INSTALLDIR)/services/etc
	install services.so $(INSTALLDIR)/services/usr/lib
	install bin/defaults.bin $(INSTALLDIR)/services/etc
ifeq ($(CONFIG_RT2880),y)
	install -d $(INSTALLDIR)/services/sbin	
	install switch $(INSTALLDIR)/services/sbin
endif

services.so: $(OBJS)
#	$(AR) arc -o $@.a $^
	$(CC) --shared $(CFLAGS) -o $@ $^ $(LDFLAGS)
#	$(CC) $(CFLAGS) -o wland $^ $(LDFLAGS)
	
services.a: $(OBJS)
	$(AR) arc -o  $@ $^

#	$(CC) -o $@.test $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
