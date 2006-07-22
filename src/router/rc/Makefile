
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

ifeq ($(CONFIG_MSSID),y)
CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
else
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS  += -s $(COPTS)
ifeq ($(ARCH),i386)
LDFLAGS	+= -s -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lcrypt -L../libnet/lib -lnet -lshared  -ldl
else
LDFLAGS	+= -s -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lcrypt -L../libnet/lib -lnet -lshared  -ldl
endif

OBJS := rc.o init.o servicemanager.o services.o stats.o crc.o http.o
OBJS += resetbutton.o ntp.o listen.o check_ps.o process_monitor.o sendudp.o radio_timer.o 

REWRITE_REVISION_H1 := $(shell echo -n '\#define SVN_REVISION "' > revision.h)
REWRITE_REVISION_H2 := $(shell svnversion -n . >> revision.h)
REWRITE_REVISION_H3 := $(shell echo '"' >> revision.h)

#ifdef $(CONFIG_DIST)
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"
#endif

ifneq ($(CONFIG_RB500),y)
OBJS += mtd.o
endif


ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif
ifeq ($(CONFIG_MMC),y)
CFLAGS += -DHAVE_MMC
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
CFLAGS += -DHAVE_MADWIFI
endif
ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
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

ifneq ($(CONFIG_RB500),y)
OBJS += wland.o
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


ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

ifeq ($(CONFIG_PPTPD),y)
OBJS += pptpd.o
CFLAGS += -DHAVE_PPTPD
endif

ifeq ($(CONFIG_L2TP),y)
CFLAGS += -DHAVE_L2TP
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
	$(STRIP) $(INSTALLDIR)/sbin/rc
	cd $(INSTALLDIR)/sbin && ln -sf rc init
	cd $(INSTALLDIR)/sbin && ln -sf rc brctl
	cd $(INSTALLDIR)/sbin && ln -sf rc erase
	cd $(INSTALLDIR)/sbin && ln -sf rc write
	cd $(INSTALLDIR)/sbin && ln -sf rc restore
	cd $(INSTALLDIR)/sbin && ln -sf rc stats
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc filter
	cd $(INSTALLDIR)/sbin && ln -sf rc resetbutton
	cd $(INSTALLDIR)/sbin && ln -sf rc filtersync
	cd $(INSTALLDIR)/sbin && ln -sf rc ntpd
	cd $(INSTALLDIR)/sbin && ln -sf rc ipupdated
	cd $(INSTALLDIR)/sbin && ln -sf rc redial
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_connect
	cd $(INSTALLDIR)/sbin && ln -sf rc hb_disconnect
	cd $(INSTALLDIR)/sbin && ln -sf rc gpio
	cd $(INSTALLDIR)/sbin && ln -sf rc listen
	cd $(INSTALLDIR)/sbin && ln -sf rc check_ps
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_success
	cd $(INSTALLDIR)/sbin && ln -sf rc process_monitor
	cd $(INSTALLDIR)/sbin && ln -sf rc site_survey
	cd $(INSTALLDIR)/sbin && ln -sf rc radio_timer
ifeq ($(CONFIG_ARP),y)
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox arp
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc sendudp
	cd $(INSTALLDIR)/sbin && ln -sf rc misc	
	cd $(INSTALLDIR)/sbin && ln -sf rc check_ses_led	
	cd $(INSTALLDIR)/sbin && ln -sf rc setpasswd
ifneq ($(CONFIG_RB500),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc wland
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc startservice
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservice
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox httpd
ifeq ($(CONFIG_BIRD),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox bird
endif
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dnsmasq
	cd $(INSTALLDIR)/usr/sbin && ln -sf /usr/sbin/iptables iptables-restore
#ifeq ($(CONFIG_UPNP),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox upnp
#endif
#ifeq ($(CONFIG_DDNS),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox ez-ipupdate
#endif
ifeq ($(CONFIG_RFLOW),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox rflow
endif
ifeq ($(CONFIG_DHCPFORWARD),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dhcpfwd
endif
#ifeq ($(CONFIG_PPPD),y)
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox pppd
#endif
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dropbearkonvert && ln -sf /bin/busybox dropbearkey && ln -sf /bin/busybox dbclient && ln -sf /bin/busybox dropbear
	mkdir -p $(INSTALLDIR)/usr/bin
	cd $(INSTALLDIR)/usr/bin && ln -sf /bin/busybox ssh && ln -sf /bin/busybox scp
endif
ifeq ($(CONFIG_PPTPD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc poptop
endif

ifeq ($(CONFIG_WOL),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc wol
endif

ifeq ($(CONFIG_DDLAN),y)
	cp check.sh $(INSTALLDIR)/sbin
endif



rc: $(OBJS)
#	$(AR) arc -o $@.a $^
	$(CC) -s -o $@ $^ $(LDFLAGS) -lshared

$(OBJS): $(CY_DEPS)
