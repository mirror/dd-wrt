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
UTILS := mtd
CFLAGS += -s -Os -mtune=mips32 -mips32 -pipe -I$(SRCBASE)/include.v23 -I$(SRCBASE)/wl/exe 

vpath %.c $(SRCBASE)/tools/misc


vpath %.c $(SRCBASE)/et/freebsd


all: $(UTILS)


clean:
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/usr/sbin
#	install -d $(INSTALLDIR)/usr/lib
	install $(UTILS) $(INSTALLDIR)/usr/sbin
	#install $(LIBS) $(INSTALLDIR)/usr/lib
#	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
ifeq ($(CONFIG_MSSID),y)
	install wl $(INSTALLDIR)/usr/sbin
#	$(STRIP) $(INSTALLDIR)/usr/sbin/wl
else
	install -D wl.v23 $(INSTALLDIR)/usr/sbin/wl
#	$(STRIP) $(INSTALLDIR)/usr/sbin/wl
endif
#	install et $(INSTALLDIR)/usr/sbin
#	$(STRIP) $(INSTALLDIR)/usr/sbin/et
	@true