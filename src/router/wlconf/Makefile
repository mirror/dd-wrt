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

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -Os -pipe -mips32 -mtune=mips32 -funit-at-a-time -DBCMWPA2
LDFLAGS	+= -L$(TOP)/nvram -lnvram -L$(TOP)/shared -lshared

ifeq ($(CONFIG_MSSID),y)
OBJS := 
#wlconf.o 
else
OBJS := 
endif
all: wlconf

clean:
	rm -f *.o

install: all
	install -d $(INSTALLDIR)/usr/sbin
ifeq ($(CONFIG_MSSID),y)
	install -D wlconf.v24 $(INSTALLDIR)/usr/sbin/wlconf
#	install wlconf $(INSTALLDIR)/usr/sbin
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