
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/ 
else
CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
endif
else
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS  += $(COPTS)
ifeq ($(ARCH),i386)
LDFLAGS	+= -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/libutils -lutils  -ldl
CFLAGS += -I$(LINUXDIR)
else
LDFLAGS	+= -L$(TOP)/nvram -I$(LINUXDIR) -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/libutils -lutils  -ldl
CFLAGS += -I$(LINUXDIR)

ifneq ($(ARCHITECTURE),lsx)
CFLAGS += -fpic -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
endif

#LDFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections

OBJS := rc.o servicemanager.o crc.o event.o
#http.o stats.o
OBJS += watchdog.o services.o mtd_main.o

REWRITE_REVISION_H1 := $(shell echo -n '\#define SVN_REVISION "' > revision.h)
REWRITE_REVISION_H2 := $(shell svnversion -n . >> revision.h)
REWRITE_REVISION_H3 := $(shell echo '"' >> revision.h)

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
ifeq ($(CONFIG_ACK),y)
CFLAGS += -DHAVE_ACK
endif
ifeq ($(CONFIG_JFFS2),y)
CFLAGS += -DHAVE_JFFS2
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
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
endif

ifeq ($(CONFIG_GEMTEK),y)
CFLAGS += -DHAVE_GEMTEK
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DDEFAULT_COUNTRY_CODE=\"$(CONFIG_DEFAULT_COUNTRYCODE)\"
endif
ifeq ($(CONFIG_OLSRD),y)
CFLAGS += -DHAVE_OLSRD
endif
ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
endif
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO
endif
ifeq ($(CONFIG_OLED),y)
CFLAGS += -DHAVE_OLED
endif
ifeq ($(CONFIG_BONDING),y)
CFLAGS += -DHAVE_BONDING
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

ifeq ($(CONFIG_MEDIASERVER),y)
CFLAGS += -DHAVE_MEDIASERVER
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI -DHAVE_ROAMING
OBJS += ../wireless-tools/libiw.so.29 
OBJS += roaming_daemon.o
endif
ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
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
ifeq ($(CONFIG_ECB3500),y)
  CFLAGS += -DHAVE_ECB3500
endif

ifeq ($(CONFIG_LS2),y)
CFLAGS += -DHAVE_LS2
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
ifeq ($(CONFIG_DANUBE),y)
CFLAGS += -DHAVE_DANUBE
endif
ifeq ($(CONFIG_STORM),y)
CFLAGS += -DHAVE_STORM
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
endif
ifeq ($(CONFIG_SAMBA_SRV),y)
CFLAGS += -DHAVE_SAMBA_SRV
endif

ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

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

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
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

ifeq ($(CONFIG_MILKFISH),y)
CFLAGS += -DHAVE_MILKFISH
endif
ifeq ($(CONFIG_RT2880),y)
CFLAGS += -DHAVE_RT2880
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
endif

vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: rc

clean:
	rm -f *.o *.a rc
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
	install init $(INSTALLDIR)/sbin	
	install check_ps $(INSTALLDIR)/sbin	
	install nvram $(INSTALLDIR)/usr/sbin	
	install ledtool $(INSTALLDIR)/sbin	
#	install event $(INSTALLDIR)/sbin	
	install radio_timer $(INSTALLDIR)/sbin	
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
	cd $(INSTALLDIR)/sbin && ln -sf rc roaming_daemon
	cd $(INSTALLDIR)/sbin && ln -sf rc get_wanface
#	cd $(INSTALLDIR)/sbin && ln -sf rc ledtool
	cd $(INSTALLDIR)/sbin && ln -sf rc write
	cd $(INSTALLDIR)/sbin && ln -sf rc restore
#	cd $(INSTALLDIR)/sbin && ln -sf rc stats
	cd $(INSTALLDIR)/sbin && ln -sf rc event
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
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
	cd $(INSTALLDIR)/sbin && ln -sf rc check_ses_led	
	cd $(INSTALLDIR)/sbin && ln -sf rc setpasswd
#	cd $(INSTALLDIR)/sbin && ln -sf rc wland
ifneq ($(CONFIG_MICRO),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
endif
ifeq ($(CONFIG_ADM5120),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc startservice
#	cd $(INSTALLDIR)/sbin && ln -sf rc switch
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservice
	cd $(INSTALLDIR)/sbin && ln -sf rc startservices
	cd $(INSTALLDIR)/sbin && ln -sf rc start_single_service
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservices
	cd $(INSTALLDIR)/sbin && ln -sf rc startstop
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
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
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /usr/sbin/dropbearmulti dropbearconvert && ln -sf /usr/sbin/dropbearmulti dropbearkey && ln -sf /usr/sbin/dropbearmulti dbclient && ln -sf /usr/sbin/dropbearmulti dropbear
	mkdir -p $(INSTALLDIR)/usr/bin
	cd $(INSTALLDIR)/usr/bin && ln -sf /usr/sbin/dropbearmulti ssh && ln -sf /usr/sbin/dropbearmulti scp
endif
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
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	$(CC) $(CFLAGS) -o ttraff ttraff.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o wland wland.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o process_monitor process_monitor.c ntp.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o resetbutton resetbutton.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o init init.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o check_ps check_ps.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o nvram nvram.c -L../nvram -lnvram
	$(CC) $(CFLAGS) -o ledtool ledtool.c $(LDFLAGS)
#	$(CC) $(CFLAGS) -o event event.c
	$(CC) $(CFLAGS) -o radio_timer radio_timer.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o listen listen.c $(LDFLAGS)

$(OBJS): $(CY_DEPS)
