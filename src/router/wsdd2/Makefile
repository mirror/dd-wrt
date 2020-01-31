#
# Makefile for wsdd2 WSD/LLMNR server
#
#	Copyright (c) 2016 NETGEAR
#	Copyright (c) 2016 Hiro Sugawara
#

OBJFILES	= wsdd2.o wsd.o llmnr.o
HEADERS		= wsdd.h wsd.h
DESTDIR		= $(INSTALLDIR)

INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
MANINSTALLDIR = $(INSTALLPREFIX)/share/man 
CFLAGS		= $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF
LDFLAGS		= $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections

all: wsdd2

wsdd2: $(OBJFILES)

$(OBJFILES) : $(HEADERS) Makefile

install:	wsdd2
	install -d $(DESTDIR)/$(SBININSTALLDIR)
	install wsdd2 $(DESTDIR)/$(SBININSTALLDIR)

clean:
	rm -f wsdd2 $(OBJFILES)
