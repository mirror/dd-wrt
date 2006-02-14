
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
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -L../../router.x86/libnet/lib -lnet -lcrypt
else
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -L../libnet/lib ../dnsmasq/src/dnsmasq.a -lnet -lcrypt 
endif

OBJS := rc.o init.o interface.o network.o services.o udhcpc.o ppp.o http.o stats.o crc.o closed/mssid.o closed/openvpn.o
OBJS += firewall.o resetbutton.o ntp.o gpio.o iptable.o ddns.o listen.o check_ps.o process_monitor.o wshaper.o wland.o mkfiles.o sendudp.o 
OBJS += ledcontrol.o

OBJS += heartbeat.o
#OBJS += eou.o
OBJS += site_suvery.o

ifneq ($(CONFIG_RB500),y)
OBJS += mtd.o
endif

ifeq ($(CONFIG_NETCONF),y)
OBJS += firewall.o
endif

ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif

ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
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

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif
ifeq ($(CONFIG_IPROUTE2),y)
CFLAGS += -DHAVE_IPROUTE2
LDFLAGS += ../iproute2/ip/ip.a ../iproute2/tc/tc.a ../iproute2/lib/libnetlink.a ../iproute2/lib/libutil.a -ldl -lm
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

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
OBJS += sshd.o
CFLAGS += -DHAVE_SSHD
LDFLAGS += -L../zlib ../dropbear/dropbear.a ../dropbear/libtomcrypt/libtomcrypt.a ../dropbear/libtommath/libtommath.a -lutil -lz  -lcrypt
#LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFWD
LDFLAGS += ../dhcpforwarder/dhcpfwd.a
endif
ifeq ($(CONFIG_PPPD),y)
CFLAGS += -DHAVE_PPPD
LDFLAGS += ../pppd/pppd/pppd.a
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
LDFLAGS += ../bird/obj/nest/all.o 
LDFLAGS += ../bird/obj/filter/all.o 
LDFLAGS += ../bird/obj/proto/bgp/all.o 
LDFLAGS += ../bird/obj/proto/ospf/all.o 
LDFLAGS += ../bird/obj/proto/pipe/all.o 
LDFLAGS += ../bird/obj/proto/rip/all.o 
LDFLAGS += ../bird/obj/proto/static/all.o 
LDFLAGS += ../bird/obj/conf/all.o
LDFLAGS += ../bird/obj/lib/birdlib.a

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
OBJS += snmp.o
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
OBJS += wol.o
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
LDFLAGS += -L$(TOP)/matrixssl/src -lmatrixsslstatic -lpthread

endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
endif

vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: rc

clean:
	rm -f *.o *.a rc
	rm -f closed/*.o rc
	rm -f *.c~

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/sbin
	#install rc $(INSTALLDIR)/sbin	
	#$(STRIP) $(INSTALLDIR)/sbin/rc
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox rc	
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox init
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox erase
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox write
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox restore
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox stats
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox hotplug
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox filter
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox resetbutton
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox filtersync
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox ntpd
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox ipupdated
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox redial
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox hb_connect
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox hb_disconnect
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox gpio
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox listen
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox check_ps
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox ddns_success
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox process_monitor
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox site_survey
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox arp
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox sendudp
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox misc	
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox check_ses_led	
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox setpasswd
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox wland
	cd $(INSTALLDIR)/sbin && ln -sf /bin/busybox restart_dns
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox httpd
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox bird
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dnsmasq
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox tc
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox ip
#	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox iptables
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
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox dhcp-fwd
endif
ifeq ($(CONFIG_PPPD),y)
	cd $(INSTALLDIR)/usr/sbin && ln -sf /bin/busybox pppd
endif
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

rc: $(OBJS)
	$(AR) arc -o $@.a $^
#	$(CC) -o $@ $^ ../httpd/httpd.a $(LDFLAGS)

$(OBJS): $(CY_DEPS)
