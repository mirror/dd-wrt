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
#CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.bcm -Wall -I$(SRCBASE)/
#else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
#endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -Os -pipe -mips32 -mtune=mips32 -funit-at-a-time -DBCMWPA2 -fpic -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS	+= -L$(TOP)/nvram -lnvram -L$(TOP)/libutils -lutils -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS += -DHAVE_BUFFALO -DBUFFALO_COUNTRY="\"$(CONFIG_COUNTRY)\""
endif
ifneq ($(CONFIG_BCM5354),y)
CFLAGS += -DHAVE_VINT
endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
OBJS := wlconf.o 
#OBJS := wlconfbcm.o 
else
OBJS := wlconf.o 
endif
else
OBJS := 
endif
all: wlconf

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

wlconf: $(OBJS)
ifeq ($(CONFIG_MSSID),y)
	$(CC) -o $@ $^ $(LDFLAGS)
else
	@true
endif