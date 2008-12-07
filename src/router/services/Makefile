

include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

LDFLAGS	= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lcrypt -L../libnet/lib -lnet -L../libutils -lutils
#ifeq ($(CONFIG_MSSID),y)
#ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
#else
#CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
#endif
LDFLAGS += -lm
#else
#CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
#endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS  += $(COPTS) -fpic -I$(TOP)/services/include
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
CFLAGS += -DHAVE_RSTATS
OBJS += rstats.o
endif
ifeq ($(CONFIG_ALLNETWRT),y)
  CFLAGS += -DHAVE_ALLNETWRT
endif
ifeq ($(CONFIG_OVERCLOCKING),y)
CFLAGS += -DHAVE_OVERCLOCKING
endif
ifeq ($(CONFIG_IPV6),y)
CFLAGS += -DHAVE_IPV6
endif
ifeq ($(CONFIG_TRIMAX),y)
CFLAGS += -DHAVE_TRIMAX
endif
ifeq ($(CONFIG_ACK),y)
CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
  CFLAGS += -DBUFFALO_JP
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CFLAGS += -DHAVE_MADWIFI_MIMO
endif
ifeq ($(CONFIG_GLAUCO),y)
CFLAGS += -DHAVE_GLAUCO
endif

ifeq ($(CONFIG_NSTX),y)
CFLAGS += -DHAVE_NSTX
OBJS += nstx.o
endif
ifeq ($(CONFIG_UDHCPD),y)
CFLAGS += -DHAVE_UDHCPD
OBJS += udhcpd.o
endif

ifeq ($(CONFIG_WPA_SUPPLICANT),y)
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
CFLAGS += -DHAVE_WPA_SUPPLICANT
OBJS += brcm_80211x.o
endif
endif
endif
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
CFLAGS += -DHAVE_WPA_SUPPLICANT
OBJS += brcm_80211x.o
endif
endif
endif
ifeq ($(CONFIG_DNSMASQ),y)
CFLAGS += -DHAVE_DNSMASQ
OBJS += dnsmasq.o
endif
ifeq ($(CONFIG_KAID),y)
CFLAGS += -DHAVE_KAID
endif
ifeq ($(CONFIG_OLED),y)
CFLAGS += -DHAVE_OLED
endif
ifeq ($(CONFIG_TONZE),y)
CFLAGS += -DHAVE_TONZE
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS += -DHAVE_SYSLOG
endif
ifeq ($(CONFIG_SPUTNIK),y)
CFLAGS += -DHAVE_SPUTNIK
endif
ifeq ($(CONFIG_MMC),y)
CFLAGS += -DHAVE_MMC
OBJS += mmc.o
endif
ifeq ($(CONFIG_PPPOESERVER),y)
CFLAGS += -DHAVE_PPPOESERVER
OBJS += pppoeserver.o
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS += -DHAVE_JFFS2
OBJS += jffs.o
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
endif

ifeq ($(CONFIG_USB),y)
CFLAGS += -DHAVE_USB
OBJS += usb_hotplug.o
endif
ifeq ($(CONFIG_ALFA_BRANDING1),y)
CFLAGS += -DHAVE_ALFA_BRANDING
endif
ifeq ($(CONFIG_ALFA_BRANDING2),y)
CFLAGS += -DHAVE_ALFA_BRANDING
endif
ifeq ($(CONFIG_USB_ADVANCED),y)
CFLAGS += -DHAVE_USB_ADVANCED
endif
ifeq ($(CONFIG_WRK54G),y)
CFLAGS += -DHAVE_WRK54G
endif
ifeq ($(CONFIG_RSTP),y)
CFLAGS += -DHAVE_RSTP
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_VLANTAGGING
endif
ifeq ($(CONFIG_GW700),y)
CFLAGS += -DHAVE_GW700
endif
ifeq ($(CONFIG_WIVIZ),y)
CFLAGS += -DHAVE_WIVIZ
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
endif
ifeq ($(CONFIG_BRANDING),y)
CFLAGS += -DCONFIG_BRANDING
endif
ifeq ($(CONFIG_THOM),y)
CFLAGS += -DHAVE_THOM
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
CFLAGS += -fpic
endif

ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif

#ifdef $(CONFIG_DIST)
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"
#endif
ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif
ifeq ($(CONFIG_SAMBA),y)
CFLAGS += -DHAVE_SAMBA
endif
ifeq ($(CONFIG_WIFIDOG),y)
CFLAGS += -DHAVE_WIFIDOG
OBJS += wifidog.o
endif
ifeq ($(CONFIG_BONDING),y)
CFLAGS += -DHAVE_BONDING
OBJS += bonding.o
endif
ifeq ($(CONFIG_RADLOCAL),y)
CFLAGS += -DHAVE_RADLOCAL
endif
ifeq ($(CONFIG_GEMTEK),y)
CFLAGS += -DHAVE_GEMTEK
endif

ifeq ($(CONFIG_KODATA),y)
CFLAGS += -DHAVE_KODATA
endif

ifneq ($(CONFIG_TW6600),y)
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
OBJS += libbridge_if.o
OBJS += libbridge_init.o
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
OBJS += libbridge_if.o
OBJS += libbridge_init.o
endif
endif

ifeq ($(CONFIG_RADIOOFF),y)
  CFLAGS += -DHAVE_RADIOOFF
endif
ifeq ($(CONFIG_MEDIASERVER),y)
CFLAGS += -DHAVE_MEDIASERVER
endif
ifeq ($(CONFIG_L2TP),y)
CFLAGS += -DHAVE_L2TP
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif
ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
OBJS += ppp.o
endif
ifeq ($(CONFIG_PPPOERELAY),y)
CFLAGS += -DHAVE_PPPOERELAY
OBJS += pppoerelay.o
endif

ifeq ($(CONFIG_DDLAN),y)
CFLAGS += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS += -DHAVE_HEARTBEAT
OBJS += heartbeat.o
endif

ifeq ($(CONFIG_RT2880),y)
OBJS += rt2880.o
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI -I../madwifi.dev/madwifi.dev -include ../madwifi.dev/madwifi.dev/include/compat.h -I../wireless-tools -DHEADERS_KERNEL

ifneq ($(CONFIG_NOWIFI),y)
OBJS += madwifi.o
OBJS += ../wireless-tools/libiw.so.29
endif
endif

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
endif
ifeq ($(CONFIG_BOESE),y)
CFLAGS += -DBOESE=1
endif
ifeq ($(CONFIG_WILLIAM),y)
CFLAGS += -DWILLIAM=1
endif

ifeq ($(CONFIG_MULTICAST),y)
CFLAGS += -DHAVE_MULTICAST
endif

ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif
ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
OBJS += upnp.o
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
CFLAGS += -DHAVE_CHILLILOCAL
OBJS += chillispot.o
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
endif

ifeq ($(CONFIG_FTP),y)
CFLAGS += -DHAVE_FTP
OBJS += proftp.o
endif
ifeq ($(CONFIG_SAMBA_SRV),y)
CFLAGS += -DHAVE_SAMBA_SRV
OBJS += sambasrv.o
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_XSCALE),y)
CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_VLANTAGGING),y)
CFLAGS += -DHAVE_PORTSETUP
endif
ifeq ($(CONFIG_CPUTEMP),y)
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_DIR400),y)
CFLAGS += -DHAVE_FONERA
CFLAGS += -DHAVE_DIR400
OBJS += icplus.o
else
ifeq ($(CONFIG_DIR300),y)
CFLAGS += -DHAVE_FONERA
CFLAGS += -DHAVE_DIR300
OBJS += icplus.o
endif
endif
ifeq ($(CONFIG_MR3202A),y)
CFLAGS += -DHAVE_MR3202A
OBJS += admtek.o
endif
ifeq ($(CONFIG_FONERA),y)
CFLAGS += -DHAVE_FONERA
endif
ifeq ($(CONFIG_MERAKI),y)
CFLAGS += -DHAVE_FONERA
CFLAGS += -DHAVE_MERAKI
endif
ifeq ($(CONFIG_EAP3660),y)
  CFLAGS += -DHAVE_EAP3660
endif
ifeq ($(CONFIG_EOC2610),y)
  CFLAGS += -DHAVE_EOC2610
endif


ifeq ($(CONFIG_NS2),y)
CFLAGS += -DHAVE_NS2
CFLAGS += -DHAVE_LS2
else
ifeq ($(CONFIG_LS2),y)
CFLAGS += -DHAVE_LS2
OBJS += icplus.o
endif
endif
ifeq ($(CONFIG_NS5),y)
CFLAGS += -DHAVE_NS5
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
ifeq ($(CONFIG_RS),y)
CFLAGS += -DHAVE_RS
endif
ifeq ($(CONFIG_DANUBE),y)
CFLAGS += -DHAVE_DANUBE
endif
ifeq ($(CONFIG_STORM),y)
CFLAGS += -DHAVE_STORM
endif
ifeq ($(CONFIG_ADM5120),y)
CFLAGS += -DHAVE_ADM5120
endif
ifeq ($(CONFIG_TW6600),y)
CFLAGS += -DHAVE_TW6600
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
ifeq ($(CONFIG_CA8),y)
CFLAGS += -DHAVE_CA8
endif
ifeq ($(CONFIG_WRT300NV2),y)
CFLAGS += -DHAVE_WRT300NV2
endif
ifeq ($(CONFIG_CAMBRIA),y)
CFLAGS += -DHAVE_CAMBRIA
endif
ifeq ($(CONFIG_WAVESAT),y)
CFLAGS += -DHAVE_WAVESAT
OBJS += wavesat.o
endif
ifeq ($(CONFIG_NOP8670),y)
CFLAGS += -DHAVE_NOP8670
CFLAGS += -DHAVE_GATEWORX
else
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_GATEWORX
ifneq ($(CONFIG_WRT300NV2),y)
CFLAGS += -DHAVE_CPUTEMP
CFLAGS += -DHAVE_VOLT
endif
endif
endif
ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
CFLAGS += -DHAVE_USB
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
ifeq ($(CONFIG_AIRNET),y)
OBJS += openvpn-airnet.o
else
OBJS += openvpn.o
endif
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
CFLAGS += -DHAVE_EOP
endif
ifeq ($(CONFIG_IPROUTE2),y)
CFLAGS += -DHAVE_IPROUTE2
endif

ifeq ($(CONFIG_ROUTERSTYLE),y)
CFLAGS += -DHAVE_ROUTERSTYLE
endif

ifeq ($(CONFIG_TELCOM),y)
CFLAGS += -DHAVE_TELCOM
endif

ifeq ($(CONFIG_GGEW),y)
CFLAGS += -DHAVE_GGEW
CFLAGS += -DHAVE_NEWMEDIA
endif

ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS += -DHAVE_NEWMEDIA
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

ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
endif

ifeq ($(CONFIG_VILIM),y)
CFLAGS += -DHAVE_VILIM
endif

ifeq ($(CONFIG_MAKSAT_BROADCOM),y)
CFLAGS += -DHAVE_MAKSAT_BROADCOM
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
OBJS += telnet.o
endif


ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DDEFAULT_COUNTRY_CODE=\"$(CONFIG_DEFAULT_COUNTRYCODE)\" -DDEFAULT_LANGUAGE=\"$(CONFIG_DEFAULT_LANGUAGE)\"
endif

ifeq ($(CONFIG_PPTP),y)
CFLAGS += -DHAVE_PPTP
endif
ifeq ($(CONFIG_PPTPD),y)
CFLAGS += -DHAVE_PPTPD
OBJS += pptp.o
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
OBJS += sshd.o
CFLAGS += -DHAVE_SSHD
endif

ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
OBJS += dhcpforward.o
CFLAGS += -DHAVE_DHCPFWD
endif
ifeq ($(CONFIG_PPPD),y)
CFLAGS += -DHAVE_PPPD
OBJS += ppp.o
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
OBJS += routing.o
endif

ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
endif

ifeq ($(CONFIG_QUAGGA),y)
CFLAGS += -DHAVE_QUAGGA
OBJS += routing.o
endif

ifeq ($(CONFIG_WSHAPER),y)
OBJS += wshaper.o
CFLAGS += -DHAVE_WSHAPER
endif

ifeq ($(CONFIG_SVQOS),y)
CFLAGS += -DHAVE_SVQOS
endif

ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
OBJS += snmp.o
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
OBJS += wol.o
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS += -DHAVE_NOCAT
OBJS += nocat.o
endif

ifeq ($(CONFIG_SER),y)
CFLAGS += -DHAVE_SER
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DHAVE_FREEBIRD
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
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
OBJS += olsrd.o
endif

ifeq ($(CONFIG_MILKFISH),y)
CFLAGS += -DHAVE_MILKFISH
OBJS += milkfish.o
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
OBJS += sputnik.o
endif

ifeq ($(CONFIG_RT2880),y)
  CFLAGS += -DHAVE_RT2880
endif
ifeq ($(CONFIG_WHRG300N),y)
  CFLAGS += -DHAVE_WHRG300N
endif
ifeq ($(CONFIG_NAS),y)
  CFLAGS += -DHAVE_NAS
endif

vpath %.c sysinit sysinit/switchlib services tools networking $(TOP)/shared $(SRCBASE)/rts/src

all: services.so services.a
ifeq ($(CONFIG_RT2880),y)
	$(CC) $(CFLAGS) -DNEED_PRINTF -o switch tools/switch.c
endif
    
clean:
	rm -f *.o *.a *.so
	rm -f *.c~ services/*.c~ networking/*.c~ tools/*.c~ sysinit/*.c~
	rm -f *.h~ services/*.h~ networking/*.h~ tools/*.h~ sysinit/*.h~

install: all
	install -d $(INSTALLDIR)/services/usr/lib	
	install services.so $(INSTALLDIR)/services/usr/lib
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
