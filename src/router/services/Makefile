

include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

LDFLAGS	= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lcrypt -L../libnet/lib -lnet -L../libutils -lutils -lwireless
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
LDFLAGS += -lm
endif
endif

BCFLAGS	= $(COPTS) -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ -I$(TOP)/httpd 
BCFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
BCFLAGS  += -fpic -I$(TOP)/services/include

CFLAGS += $(BCFLAGS)

HCFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
HCFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
HCFLAGS  += -fpic -I$(TOP)/services/include


OBJS = services.o network.o firewall.o mssid.o wshaper.o ddns.o devinit.o sysinit.o sysinit-$(ARCHITECTURE).o interface.o udhcpc.o mkfiles.o iptable.o defaults.o \
       bridgetools.o igmp.o wpa.o vlantagging.o httpd.o setuserpw.o anchorfree.o gpio.o mask.o reset_defaults.o vpn.o cron.o syslog.o redial.o radvd.o	\
       pppoe.o pptpclient.o l2tp.o hwmon.o ttraff.o radiotimer.o processmonitor.o wland.o radiofunctions.o apserv.o aoss.o processmanager.o utils.o


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
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),EU)
  CFLAGS_EXTRA += -DBUFFALO_EU
endif
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),US)
  CFLAGS_EXTRA += -DBUFFALO_US
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS_EXTRA += -DHAVE_MADWIFI_MIMO
OBJS += madwifi_11n.o
endif
ifeq ($(CONFIG_ATH9K),y)
CFLAGS_EXTRA += -DHAVE_ATH9K
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
ifeq ($(CONFIG_LAGUNA),y)
CFLAGS_EXTRA += -DHAVE_LAGUNA
CFLAGS_EXTRA += -DHAVE_CPUTEMP
CFLAGS_EXTRA += -DHAVE_VOLT
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
ifeq ($(CONFIG_DLM101),y)
  CFLAGS += -DHAVE_DLM101
endif
ifeq ($(CONFIG_W502U),y)
  CFLAGS += -DHAVE_W502U
endif
ifeq ($(CONFIG_RT3052),y)
  CFLAGS += -DHAVE_RT3052
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
ifeq ($(CONFIG_DIR615E),y)
  CFLAGS_EXTRA += -DHAVE_DIR615E
endif

ifeq ($(CONFIG_DIR600),y)
  CFLAGS_EXTRA += -DHAVE_DIR600
endif
ifeq ($(CONFIG_RT10N),y)
  CFLAGS_EXTRA += -DHAVE_RT10N
endif
ifeq ($(CONFIG_NP25G),y)
  CFLAGS_EXTRA += -DHAVE_NP25G
endif
ifeq ($(CONFIG_WPE53G),y)
  CFLAGS_EXTRA += -DHAVE_WPE53G
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
ifeq ($(CONFIG_HOTPLUG2),y)
CFLAGS_EXTRA += -DHAVE_HOTPLUG2
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
ifeq ($(CONFIG_NTFS3G),y)
CFLAGS_EXTRA += -DHAVE_NTFS3G
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
ifeq ($(CONFIG_WRT160NL),y)
CFLAGS_EXTRA += -DHAVE_WRT160NL
endif
ifeq ($(CONFIG_TG2521),y)
CFLAGS_EXTRA += -DHAVE_TG2521
endif
ifeq ($(CONFIG_WR1043),y)
CFLAGS_EXTRA += -DHAVE_WR1043
endif
ifeq ($(CONFIG_USR8200),y)
CFLAGS_EXTRA += -DHAVE_USR8200
endif
ifeq ($(CONFIG_WR941),y)
CFLAGS_EXTRA += -DHAVE_WR941
endif
ifeq ($(CONFIG_WR741),y)
CFLAGS_EXTRA += -DHAVE_WR741
endif
ifeq ($(CONFIG_P910ND),y)
CFLAGS_EXTRA += -DHAVE_P910ND
OBJS += printer.o
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS_EXTRA += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_WZRHPAG300NH),y)
CFLAGS_EXTRA += -DHAVE_WZRHPAG300NH
endif
ifeq ($(CONFIG_AP96),y)
CFLAGS_EXTRA += -DHAVE_AP96
endif
ifeq ($(CONFIG_WPS),y)
CFLAGS_EXTRA += -DHAVE_WPS
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS_EXTRA += -DHAVE_WHRHPGN
endif
ifeq ($(CONFIG_TEW632BRP),y)
CFLAGS_EXTRA += -DHAVE_TEW632BRP
endif
ifeq ($(CONFIG_WZRG300NH),y)
CFLAGS_EXTRA += -DHAVE_WZRG300NH
endif
ifeq ($(CONFIG_DIR825),y)
CFLAGS_EXTRA += -DHAVE_DIR825
endif
ifeq ($(CONFIG_WNDR3700),y)
CFLAGS_EXTRA += -DHAVE_WNDR3700
endif
ifeq ($(CONFIG_JA76PF),y)
CFLAGS_EXTRA += -DHAVE_JA76PF
endif
ifeq ($(CONFIG_JWAP003),y)
CFLAGS_EXTRA += -DHAVE_JWAP003
endif
ifeq ($(CONFIG_ALFAAP94),y)
CFLAGS_EXTRA += -DHAVE_ALFAAP94
endif
ifeq ($(CONFIG_WRT400),y)
CFLAGS_EXTRA += -DHAVE_WRT400
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
ifeq ($(CONFIG_AP_SERV),y)
CFLAGS_EXTRA += -DHAVE_AP_SERV
endif
ifeq ($(CONFIG_AOSS),y)
CFLAGS_EXTRA += -DHAVE_AOSS
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
ifeq ($(CONFIG_MADWIFI_MIMO),y)
NOBJS += site_survey_madwifi_11n.o
NFILES += tools/site_survey_madwifi_11n.c
NCFLAGS += $(BCFLAGS) -DHAVE_MADWIFI -DMADWIFI_MIMO -I../madwifi.dev/madwifi_mimo.dev/core -include ../madwifi.dev/madwifi_mimo.dev/core/include/compat.h -I../wireless-tools -DHEADERS_KERNEL -DATH_USE_COMMONHAL
endif
ifeq ($(CONFIG_ATH9K),y)
ATH9KCFLAGS+= $(COPTS) -fPIC -DHAVE_ATH9K -I. -I$(TOP)/services/include -I$(TOP)/shared -I../httpd -I$(SRCBASE)/include.v24 -I$(TOP)/compat-wireless/include -I$(TOP)/libnl-tiny/include -I$(SRCBASE)/
ATH9KOBJS += madwifi_ath9k.o
ATH9KFILES += networking/madwifi_ath9k.c
ATH9KLDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
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
ifeq ($(CONFIG_RB600),y)
CFLAGS_EXTRA += -fpic
endif

ifeq ($(CONFIG_ESR9752),y)
CFLAGS_EXTRA += -DHAVE_ESR9752
endif
ifeq ($(CONFIG_ACXNR22),y)
CFLAGS_EXTRA += -DHAVE_ACXNR22
endif
ifeq ($(CONFIG_MI424WR),y)
CFLAGS_EXTRA += -DHAVE_MI424WR
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
ifeq ($(CONFIG_SOLO51),y)
  CFLAGS_EXTRA += -DHAVE_SOLO51
OBJS += icplus.o
endif

ifeq ($(CONFIG_WG302),y)
  CFLAGS_EXTRA += -DHAVE_WG302
endif
ifeq ($(CONFIG_WG302V1),y)
  CFLAGS_EXTRA += -DHAVE_WG302V1
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS_EXTRA += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL -DATH_USE_COMMONHAL
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
ifeq ($(CONFIG_SAMBA3),y)
CFLAGS_EXTRA += -DHAVE_SAMBA3
OBJS += samba3.o
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
ifeq ($(CONFIG_RTG32),y)
CFLAGS_EXTRA += -DHAVE_RTG32
endif
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS_EXTRA += -DHAVE_MAGICBOX
CFLAGS_EXTRA += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_RB600),y)
CFLAGS_EXTRA += -DHAVE_RB600
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
ifeq ($(CONFIG_R2M),y)
  CFLAGS_EXTRA += -DHAVE_R2M
endif
ifeq ($(CONFIG_R5M),y)
  CFLAGS_EXTRA += -DHAVE_R5M
endif
ifeq ($(CONFIG_BS2MHP),y)
  CFLAGS_EXTRA += -DHAVE_BS2MHP
endif
ifeq ($(CONFIG_BS5MHP),y)
  CFLAGS_EXTRA += -DHAVE_BS5MHP
endif
ifeq ($(CONFIG_NS2M),y)
  CFLAGS_EXTRA += -DHAVE_NS2M
endif
ifeq ($(CONFIG_NS5M),y)
  CFLAGS_EXTRA += -DHAVE_NS5M
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
ifeq ($(CONFIG_WIKINGS),y)
CFLAGS_EXTRA += -DHAVE_WIKINGS
endif
ifeq ($(CONFIG_NEXTMEDIA),y)
CFLAGS_EXTRA += -DHAVE_NEXTMEDIA
endif
ifeq ($(CONFIG_CARLSONWIRELESS),y)
CFLAGS_EXTRA += -DHAVE_CARLSONWIRELESS
endif
ifeq ($(CONFIG_UBNTM),y)
CFLAGS_EXTRA += -DHAVE_UBNTM
endif
ifeq ($(CONFIG_WHRHPGN),y)
CFLAGS_EXTRA += -DHAVE_WHRHPGN
endif

ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS_EXTRA += -DHAVE_BCMMODERN
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
ifeq ($(CONFIG_AP83),y)
CFLAGS_EXTRA += -DHAVE_AP83
endif
ifeq ($(CONFIG_AP94),y)
CFLAGS_EXTRA += -DHAVE_AP94
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
ifeq ($(CONFIG_WBD222),y)
CFLAGS_EXTRA += -DHAVE_WBD222
endif
ifeq ($(CONFIG_OPENRISC),y)
CFLAGS_EXTRA += -DHAVE_OPENRISC
endif
ifeq ($(CONFIG_ADM5120),y)
CFLAGS_EXTRA += -DHAVE_ADM5120
endif

ifneq ($(ARCHITECTURE),broadcom)
ifeq ($(CONFIG_RELAYD),y)
CFLAGS_EXTRA += -DHAVE_RELAYD
endif
ifeq ($(CONFIG_MADWIFI),y)
CFLAGS_EXTRA += -DHAVE_RELAYD
endif

ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS_EXTRA += -DHAVE_RELAYD
endif
endif
ifeq ($(CONFIG_WP54G),y)
CFLAGS_EXTRA += -DHAVE_WP54G
endif
ifeq ($(CONFIG_WP543),y)
CFLAGS_EXTRA += -DHAVE_WP543
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
ifeq ($(CONFIG_MYNETWAY),y)
CFLAGS_EXTRA += -DHAVE_MYNETWAY
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

ifeq ($(CONFIG_FREERADIUS),y)
CFLAGS_EXTRA += -DHAVE_FREERADIUS
OBJS += freeradius.o
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
ifeq ($(CONFIG_HOTSPOT),y)
CFLAGS_EXTRA += -DHAVE_HOTSPOT
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
ifeq ($(CONFIG_WR5422),y)
  CFLAGS_EXTRA += -DHAVE_WR5422
endif
ifeq ($(CONFIG_AR670W),y)
  CFLAGS_EXTRA += -DHAVE_AR670W
endif
ifeq ($(CONFIG_AR690W),y)
  CFLAGS_EXTRA += -DHAVE_AR690W
endif
ifeq ($(CONFIG_BR6574N),y)
  CFLAGS_EXTRA += -DHAVE_BR6574N
endif
ifeq ($(CONFIG_ESR6650),y)
  CFLAGS_EXTRA += -DHAVE_ESR6650
endif
ifeq ($(CONFIG_EAP9550),y)
  CFLAGS_EXTRA += -DHAVE_EAP9550
endif
ifeq ($(CONFIG_NAS),y)
  CFLAGS_EXTRA += -DHAVE_NAS
endif
ifeq ($(CONFIG_NVRAM_60K),y)
  CFLAGS_EXTRA += -DHAVE_NVRAM_60K
endif
ifeq ($(CONFIG_ADMTEKNESTEDVLAN),y)
  CFLAGS_EXTRA += -DHAVE_ADMTEKNESTEDVLAN
endif

CFLAGS += $(CFLAGS_EXTRA)
HCFLAGS += $(CFLAGS_EXTRA)

vpath %.c sysinit sysinit/switchlib services tools networking utils $(TOP)/shared $(SRCBASE)/rts/src

all: services.so services.a
ifeq ($(CONFIG_RT2880),y)
	$(CC) $(CFLAGS) -DNEED_PRINTF -o switch tools/switch.c
	$(CC) $(CFLAGS) -DNEED_PRINTF -o mii_mgr tools/mii_mgr.c
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
	install mii_mgr $(INSTALLDIR)/services/sbin
endif

services.so: $(OBJS)
#	$(AR) arc -o $@.a $^
ifeq ($(CONFIG_MADWIFI_MIMO),y)
	$(CC) --shared -c $(NCFLAGS) -o $(NOBJS) $(NFILES)
endif
ifeq ($(CONFIG_ATH9K),y)
	$(CC) -shared -c $(ATH9KCFLAGS) $(ATH9KFILES)
endif
	$(CC) --shared $(CFLAGS) -o $@ $^  $(NOBJS) $(ATH9KOBJS) $(LDFLAGS) $(ATH9KLDFLAGS)
#	$(CC) $(CFLAGS) -o wland $^ $(LDFLAGS)
	
services.a: $(OBJS)
	$(AR) arc -o  $@ $^

#	$(CC) -o $@.test $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
