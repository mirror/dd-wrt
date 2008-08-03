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
UTILS = epi_ttcp
endif
endif

CFLAGS += -s $(COPTS) -I../shared -I$(SRCBASE)/include.bcm -I$(SRCBASE)/wl/exe -I$(LINUXDIR) -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS += -L../nvram -L../shared -lshared -L../libutils -lutils -lnvram -ffunction-sections -fdata-sections -Wl,--gc-sections

vpath %.c $(SRCBASE)/tools/misc


vpath %.c $(SRCBASE)/et/freebsd
ifeq ($(ARCHITECTURE),gateworx)
UTILS += showmac setmac fischecksum
endif
ifeq ($(CONFIG_MADWIFI),y)
UTILS += wl_atheros
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
all:
	$(CC) -o mtd -I$(LINUXDIR)/include $(COPTS) mtd_arm.c
	$(CC) -o epi_ttcp -I$(LINUXDIR)/include $(COPTS) mtd_arm.c
#	$(CC) -o jffs2root -I$(LINUXDIR)/include $(COPTS) jffs2root.c	
else
all: $(UTILS)
ifeq ($(CONFIG_BUFFALO),y)
	make -C src
else
	if test -e "src/Makefile"; then make -C src; fi
endif
endif

clean:
ifeq ($(CONFIG_BUFFALO),y)
	make -C src clean
else
	if test -e "src/Makefile"; then make -C src clean; fi
endif
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/utils/usr/sbin
#	install -d $(INSTALLDIR)/utils/usr/lib
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	install $(UTILS) $(INSTALLDIR)/utils/usr/sbin
endif
endif
	#install $(LIBS) $(INSTALLDIR)/utils/usr/lib
#	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
ifeq ($(ARCH),armeb)
	install -d $(INSTALLDIR)/utils/sbin
#	install jffs2root $(INSTALLDIR)/sbin	
else

ifneq ($(CONFIG_MADWIFI),y)

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
	install -D wl.bcm $(INSTALLDIR)/utils/usr/sbin/wl
else

ifneq ($(CONFIG_BCM5354),y)
	install -D wl $(INSTALLDIR)/utils/usr/sbin/wl
else
	install -D wl $(INSTALLDIR)/utils/usr/sbin/wl
endif
endif
else
	install -D wl.v23 $(INSTALLDIR)/utils/usr/sbin/wl
endif

endif
	@true
endif