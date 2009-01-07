#
# Linux netconf Makefile
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.2 2004/11/16 08:45:03 honor Exp $
#
include $(TOP)/.config

ifeq ($(CONFIG_MSSID),y)
CFLAGS	= -I. -I$(TOP)/shared -I$(TOP)/iptables.linksys/include -I$(SRCBASE)/include.v24 -I$(LINUXDIR)/include -Wall
else
CFLAGS	= -I. -I$(TOP)/shared -I$(TOP)/iptables.linksys/include -I$(SRCBASE)/include.v23 -I$(LINUXDIR)/include -Wall
endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS = -L. -L$(TOP)/iptables.linksys/libiptc/

ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -fpic
endif

all: libnetconf.so libnetconf.a netconf 

install: all
	install -D libnetconf.so $(INSTALLDIR)/usr/lib/libnetconf.so
#$(STRIP) $(INSTALLDIR)/usr/lib/libnetconf.so

clean:
	rm -f *.o *.a *.so netconf

libnetconf.so: netconf_linux.o
	$(LD) -shared -o $@ $^ $(LDFLAGS) -liptc

libnetconf.a: netconf_linux.o
	 $(AR) arc -o $@ $^

netconf: main.o libnetconf.so
	$(CC) -o $@ $< $(LDFLAGS) -lnetconf
