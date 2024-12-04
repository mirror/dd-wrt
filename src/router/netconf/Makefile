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

#CFLAGS = -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include
#CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/iptables/include -I$(SRCBASE)/include.v24 -I$(LINUXDIR)/include -Wall
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/iptables-new/include -I$(SRCBASE)/include.v24  -Wall -I.
CFLAGS += $(COPTS) $(LTO) -DLINUX26 -ffunction-sections -fdata-sections -Wl,--gc-sections  -Wno-incompatible-pointer-types
CFLAGS += $(MIPS16_OPT)  -DIPTCSTATIC= -DNEED_PRINTF

CFLAGS += -DLINUX_2_6_36

#LDFLAGS = -L. -L$(TOP)/iptables/libiptc/ -liptc
LDFLAGS = -L. -L$(TOP)/iptables-new/libiptc/ -lip4tc

all: libnetconf.a 

install: all
#	install -D libnetconf.so $(INSTALLDIR)/usr/lib/libnetconf.so
#$(STRIP) $(INSTALLDIR)/usr/lib/libnetconf.so
	@true

clean:
	rm -f *.o *.a *.so netconf

libnetconf.so: netconf_linux.o
	$(LD) -shared -o $@ $^ $(LDFLAGS) -liptc

libnetconf.a: netconf_linux.o
	 $(AR) rcu  $(LTOPLUGIN) -o $@ $^

netconf: main.o libnetconf.a
	$(CC) -o $@ $< $(LDFLAGS) -lnetconf
