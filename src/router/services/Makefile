
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

LDFLAGS	= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lcrypt -L../libnet/lib -lnet
ifeq ($(CONFIG_MSSID),y)
CFLAGS	= -DHAVE_MSSID -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ 
LDFLAGS += -lm
else
CFLAGS	= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ 
endif
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/rc -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS  += $(COPTS) -fpic

OBJS = services.o openvpn.o network.o firewall.o mssid.o wshaper.o ddns.o sysinit.o sysinit-$(ARCHITECTURE).o interface.o udhcpc.o ledcontrol.o mkfiles.o iptable.o defaults.o stabridge.o

ifneq ($(CONFIG_XSCALE),y)
OBJS += gpio.o
endif

ifeq ($(CONFIG_RSTATS),y)
CFLAGS += -DHAVE_RSTATS
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
endif
ifeq ($(CONFIG_BRANDING),y)
CFLAGS += -DCONFIG_BRANDING
endif
ifeq ($(CONFIG_MADWIFI),y)
OBJS += site_survey_madwifi.o
else
OBJS += site_survey_broadcom.o
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
endif
ifeq ($(CONFIG_GEMTEK),y)
CFLAGS += -DHAVE_GEMTEK
endif

ifeq ($(CONFIG_KODATA),y)
CFLAGS += -DHAVE_KODATA
endif
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
OBJS += libbridge_if.o
OBJS += libbridge_init.o

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
endif

ifeq ($(CONFIG_DDLAN),y)
CFLAGS += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS += -DHAVE_HEARTBEAT
OBJS += heartbeat.o
endif

ifeq ($(CONFIG_MADWIFI),y)
OBJS += madwifi.o
CFLAGS += -DHAVE_MADWIFI -I../madwifi -include ../madwifi/include/compat.h -I../wireless-tools -DHEADERS_KERNEL
OBJS += ../wireless-tools/libiw.so.29


endif
ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
endif

ifeq ($(CONFIG_MULTICAST),y)
CFLAGS += -DHAVE_MULTICAST
endif

ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif
ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
CFLAGS += -DHAVE_CHILLILOCAL
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_XSCALE),y)
CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_PORTSETUP),y)
CFLAGS += -DHAVE_PORTSETUP
endif
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_GATEWORX
CFLAGS += -DHAVE_CPUTEMP
CFLAGS += -DHAVE_VOLT
endif
ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
CFLAGS += -DHAVE_USBHOTPLUG
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
CFLAGS += -DHAVE_EOP
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

ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DHAVE_MAKSAT
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

ifeq ($(CONFIG_PPTP),y)
CFLAGS += -DHAVE_PPTP
endif
ifeq ($(CONFIG_PPTPD),y)
CFLAGS += -DHAVE_PPTPD
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
endif

ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
endif

ifeq ($(CONFIG_ZEBRA),y)
CFLAGS += -DHAVE_ZEBRA
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

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
endif

vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: services.so services.a

clean:
	rm -f *.o *.a *.so
	rm -f *.c~

install: all
	install -d $(INSTALLDIR)/usr/lib	
	install services.so $(INSTALLDIR)/usr/lib

services.so: $(OBJS)
#	$(AR) arc -o $@.a $^
	$(CC) --shared -o $@ $^ $(LDFLAGS)
	
services.a: $(OBJS)
	$(AR) arc -o  $@ $^

#	$(CC) -o $@.test $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
