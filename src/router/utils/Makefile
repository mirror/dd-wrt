#
# Miscellaneous utilities Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.5 2003/10/29 03:43:55 honor Exp $
#
ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

include $(TOP)/.config

#LIBS := libbb.so libbcm.so libbcmcrypto.so
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
#UTILS = epi_ttcp
endif
endif

CFLAGS += $(MIPS16_OPT) $(COPTS) -I../shared -I$(SRCBASE)/include.bcm -I$(SRCBASE)/wl/exe -I$(LINUXDIR) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(LINUXDIR)/brcm/shared/bcmwifi/include -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
LDFLAGS += -L../nvram -L../shared -L../libutils -lutils -lshutils -lnvram -lwireless -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(CONFIG_ATH9K),y)
LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
endif

vpath %.c $(SRCBASE)/tools/misc


vpath %.c $(SRCBASE)/et/freebsd
ifeq ($(ARCHITECTURE),gateworx)
UTILS += showmac setmac fischecksum
endif


ifeq ($(CONFIG_ATH9K),y)
CFLAGS += -DHAVE_ATH9K
endif
ifeq ($(CONFIG_MADWIFI),y)
UTILS += wl_atheros
else
ifeq ($(CONFIG_MADWIFI_MIMO),y)
UTILS += wl_atheros
endif
endif
ifeq ($(CONFIG_RT2880),y)
UTILS += wl_rt2880
endif
ifeq ($(CONFIG_RT61),y)
UTILS += wl_rt2880
endif
ifeq ($(CONFIG_CA8),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_FONERA),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_LS2),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_LS5),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_DIST),"micro")
CFLAGS += -DHAVE_MICRO
endif
ifeq ($(CONFIG_DIST),"micro-special")
CFLAGS += -DHAVE_MICRO
endif
ifeq ($(CONFIG_SYSLOG),y)
CFLAGS += -DHAVE_SYSLOG
endif

ifeq ($(CONFIG_MERAKI),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_WHRAG108),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_TW6600),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_PB42),y)
UTILS += fischecksum
endif
ifeq ($(CONFIG_LSX),y)
UTILS += fischecksum
endif
#ifeq ($(CONFIG_XSCALE),y)
#UTILS += gpio
#endif

ifeq ($(ARCHITECTURE),newmedia)
all: clean
	$(CC) -o mtd -I$(LINUXDIR)/include $(COPTS) mtd_arm.c
	$(CC) -o epi_ttcp -I$(LINUXDIR)/include $(COPTS) mtd_arm.c
#	$(CC) -o jffs2root -I$(LINUXDIR)/include $(COPTS) jffs2root.c	
else
all: clean $(UTILS)
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
ifneq ($(CONFIG_RT61),y)
	if test -e "src.new/Makefile"; then make -C src.new; fi
endif
endif
endif
endif
ifeq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_MADWIFI_MIMO),y)
#	make -C wprobe
endif
endif




clean:
	if test -e "src.new/Makefile"; then make -C src.new clean; fi
ifeq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_MADWIFI_MIMO),y)
#	make -C wprobe clean
endif
endif
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/utils/usr/sbin
#	install -d $(INSTALLDIR)/utils/usr/lib
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifneq ($(UTILS),)
	install $(UTILS) $(INSTALLDIR)/utils/usr/sbin
endif
endif
endif
	#install $(LIBS) $(INSTALLDIR)/utils/usr/lib
#	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
ifeq ($(ARCH),armeb)
	install -d $(INSTALLDIR)/utils/sbin
#	install jffs2root $(INSTALLDIR)/sbin	
else

ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_RT2880),y)
ifeq ($(CONFIG_BCMMODERN),y)
ifeq ($(PLATFORM),arm)
	install -D wl.modern.arm $(INSTALLDIR)/utils/usr/sbin/wl
else
	install -D wl.modern $(INSTALLDIR)/utils/usr/sbin/wl
endif
	chmod +s $(INSTALLDIR)/utils/usr/sbin/wl
else
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BCMMODERN),y)
ifeq ($(PLATFORM),arm)
	install -D wl.modern.arm $(INSTALLDIR)/utils/usr/sbin/wl
else
	install -D wl.modern $(INSTALLDIR)/utils/usr/sbin/wl
endif
	chmod +s $(INSTALLDIR)/utils/usr/sbin/wl
else
ifeq ($(CONFIG_BUFFALO),y)
	install -D wl.bcm $(INSTALLDIR)/utils/usr/sbin/wl
else
	install -D wl $(INSTALLDIR)/utils/usr/sbin/wl
endif
endif
else
	install -D wl.v23 $(INSTALLDIR)/utils/usr/sbin/wl
endif


endif
endif
ifeq ($(CONFIG_MADWIFI),y)
	cd $(INSTALLDIR)/utils/usr/sbin && ln -s wl_atheros wl
ifneq ($(CONFIG_MADWIFI_MIMO),y)
#	install -D wprobe/wprobe-util $(INSTALLDIR)/utils/usr/sbin/wprobe-util
endif
endif
	@true
endif
endif
