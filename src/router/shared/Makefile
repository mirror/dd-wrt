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
CFLAGS	= -I. -I../httpd -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
else
CFLAGS	= -I. -I../httpd -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/
endif

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -fpic
LDFLAGS += -L.
OBJS := nvparse.o shutils.o wl.o wl_linux.o linux_timer.o utils.o nvports.o 
#defaults.o
ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -fpic

endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
  CFLAGS += -DHAVE_DROPBEAR -DHAVE_SSHD
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
OBJS += register.o
endif

ifeq ($(CONFIG_SILENCE),y)
  CFLAGS += -DHAVE_SILENCE
endif
ifeq ($(CONFIG_SAMBA),y)
  CFLAGS += -DHAVE_SAMBA
endif
ifeq ($(CONFIG_GEMTEK),y)
  CFLAGS += -DHAVE_GEMTEK
endif

ifeq ($(CONFIG_RAMSKOV),y)
  CFLAGS += -DHAVE_RAMSKOV
endif

ifeq ($(CONFIG_SNMP),y)
  CFLAGS += -DHAVE_SNMP
endif

ifeq ($(REGDOMAIN_OVERRIDE),y)
CFLAGS += -DREGDOMAIN_OVERRIDE
endif

ifeq ($(CONFIG_RB500),y)
  CFLAGS += -DHAVE_RB500
endif

ifeq ($(CONFIG_XSCALE),y)
  CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_GATEWORX),y)
  CFLAGS += -DHAVE_GATEWORX
endif
ifeq ($(CONFIG_X86),y)
  CFLAGS += -DHAVE_X86
endif
ifeq ($(CONFIG_MAGICBOX),y)
  CFLAGS += -DHAVE_MAGICBOX
endif

ifeq ($(CONFIG_CHILLILOCAL),y)
  CFLAGS += -DHAVE_CHILLILOCAL
endif

ifeq ($(CONFIG_PPPOERELAY),y)
  CFLAGS += -DHAVE_PPPOERELAY
endif

ifeq ($(CONFIG_CHILLISPOT),y)
  CFLAGS += -DHAVE_CHILLI
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

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI -I../services -I../madwifi -include ../madwifi/include/compat.h -I../wireless-tools -DHEADERS_KERNEL
OBJS += ../wireless-tools/libiw.so.29 
endif

ifeq ($(CONFIG_NEWMEDIA),y)
  CFLAGS += -DHAVE_NEWMEDIA
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

ifeq ($(CONFIG_CDEBUG),y)
  CFLAGS += -DCDEBUG
endif

ifeq ($(CONFIG_MSSID),y)
  CFLAGS += -DHAVE_MSSID
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
  CFLAGS += -DSET_BOOT_WAIT
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
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

ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

ifeq ($(CONFIG_MATRIXSSL),y)
#CFLAGS += -I$(TOP)/matrixssl
#LDFLAGS += -L$(TOP)/matrixssl/src -lmatrixssl
CFLAGS += -DHAVE_MATRIXSSL
endif
HFLAGS := $(CFLAGS)
CFLAGS += $(COPTS)
all: libshared.so libshared.a
#	gcc -o defaults $(HFLAGS) defaults.c
#	./defaults

install: all
ifeq ($(CONFIG_XSCALE),y)
	install -d $(INSTALLDIR)/lib
	install -m 755 libshared.so $(INSTALLDIR)/lib
else
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 libshared.so $(INSTALLDIR)/usr/lib
endif
#	install defaults.bin $(INSTALLDIR)/etc
	$(STRIP) $(INSTALLDIR)/lib/libshared.so

clean:
	rm -f *.o *.so
	rm -f *.a
	rm -f *.h~
	rm -f *.c~

libshared.so: $(OBJS)
#	$(CC) -shared -o $@ $(CFLAGS) $(OBJS) 
	$(LD) -shared -o $@ $^ -L../nvram -lnvram

libshared.a: $(OBJS)
	$(AR) arc -o  $@ $^

*.o: $(CY_DEPS)
