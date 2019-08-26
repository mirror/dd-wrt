#
# Router Wireless Interface Configuration Utility Makefile
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.                
#                                     
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior      
# written permission of Broadcom Corporation.                            
#
# $Id: Makefile,v 1.2 2004/06/25 02:31:56 honor Exp $
#
include $(TOP)/.config

#ifeq ($(CONFIG_BUFFALO),y)
#else
#endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -DBCMWPA2 -fpic -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS	+= -L$(TOP)/nvram -lnvram -lwireless -L$(TOP)/libutils -lutils -lshutils -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DBUFFALO_COUNTRY="\"$(CONFIG_COUNTRY)\""
endif
ifeq ($(CONFIG_NORTHSTAR_NOSMP),y)
CFLAGS += -DHAVE_600
endif
CFLAGS += $(MIPS16_OPT)
LDFLAGS += $(MIPS16_OPT)
ifeq ($(CONFIG_QTN),y)
ADDLIBS += -Wl,-rpath,$(TOP)/qtn/libqcsapi_client 
endif



ifneq ($(CONFIG_BCM5354),y)
CFLAGS += -DHAVE_VINT
endif
ifeq ($(CONFIG_DHDAP),y)
CFLAGS += -D__CONFIG_DHDAP__
endif

ifeq ($(CONFIG_80211AC),y)
OBJS := wlconf_3.x.c 
CFLAGS	+= -I. -I$(TOP)/shared -Wall -I$(SRCBASE)/
CFLAGS +=  -I$(LINUXDIR)/brcm/$(ARCH)/include -I$(LINUXDIR)/brcm/$(ARCH)/common/include  -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include
else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
CFLAGS	+= -I. -I$(TOP)/shared -I$(LINUXDIR)/brcm/include -Wall -I$(SRCBASE)/ -I$(LINUXDIR)/brcm/shared/bcmwifi/include

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
OBJS := wlconf.c
#OBJS := wlconfbcm.o 
else
OBJS := wlconf.c 
endif
else
OBJS := 
endif

endif
all:
ifeq ($(CONFIG_MSSID),y)
	$(CC) -o wlconf $(OBJS) $(CFLAGS) $(LDFLAGS) $(ADDLIBS)
else
	@true
endif
clean:
	rm -f *.o

install: all
	install -d $(INSTALLDIR)/usr/sbin
ifeq ($(CONFIG_MSSID),y)
	install wlconf $(INSTALLDIR)/usr/sbin
else
	install -D wlconf.v23 $(INSTALLDIR)/usr/sbin/wlconf
endif
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf

wlconf: 
ifeq ($(CONFIG_MSSID),y)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(ADDLIBS)
else
	@true
endif