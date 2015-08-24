#
# Linux NVRAM Makefile
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


include $(TOP)/.config

CFLAGS	+= $(COPTS) -Ilibubox -I. -D_GNU_SOURCE -Wall -Wno-gnu -std=c99 -Dtypeof=__typeof

vpath %.c libubox src

all: omcproxy

install: all
	install -D omcproxy $(INSTALLDIR)/usr/sbin/omcproxy

clean:
	rm -f *.o omcproxy

omcproxy: uloop.o avl.o blobmsg.o blob.o client.o mrib.o querier.o groups.o igmp.o mld.o proxy.o omcproxy.o
	$(CC) $(CFLAGS) -o $@ $^

