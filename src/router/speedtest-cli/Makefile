#
# Speedtest-cli
# 
# Copyright (C) 2015, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: $
#

CFLAGS += -I. -I$(TOP)/shared -Wall
CFLAGS += $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/curl/include $(LTO)

LDFLAGS	+= $(LDLTO)
LDFLAGS += $(EXTRA_LDFLAGS)
LDFLAGS	+= -lpthread -lm -L$(TOP)/curl/build_static/lib/.libs -lcurl -L$(TOP)/bearssl/build -lbearssl -mthumb

all: speedtest_cli

clean:
	rm -f *.o speedtest_cli

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install speedtest_cli $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/speedtest_cli

speedtest_cli: speedtest_cli.o ping.o
	$(CC) -o speedtest_cli speedtest_cli.o ping.o $(LDFLAGS)
