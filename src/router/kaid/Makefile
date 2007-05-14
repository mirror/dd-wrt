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
#CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I$(SRCBASE)/
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s $(COPTS)
#LDFLAGS	+= -L$(TOP)/nvram -lnvram -L$(TOP)/shared -lshared
ifneq ($(CONFIG_LIBCPP),y)
export PRE_CFLAGS:= -static
export STATIC:= -static
endif


all:
	make -C src/pthreadcc
	make -C src/socketcc
	make -C src

install: all
	#install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/etc/kaid
	install -d $(INSTALLDIR)/etc/config
	#install kaid $(INSTALLDIR)/usr/sbin
	install kaid.conf $(INSTALLDIR)/etc/kaid
	install config/kaid.webservices $(INSTALLDIR)/etc/config/kaid.webservices
	install config/kaid.nvramconfig $(INSTALLDIR)/etc/config/kaid.nvramconfig
	install config/kaid.startup $(INSTALLDIR)/etc/config/kaid.startup


clean: 
	make -C src/pthreadcc clean
	make -C src/socketcc clean
	make -C src clean
