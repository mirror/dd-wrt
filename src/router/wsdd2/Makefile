#
# Makefile for wsdd2 WSD/LLMNR server
#
#	Copyright (c) 2016 NETGEAR
#	Copyright (c) 2016 Hiro Sugawara
#

CFLAGS		= $(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF
LDFLAGS		= $(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections

OBJFILES      = wsdd2.o wsd.o llmnr.o
HEADERS       = wsdd.h wsd.h

PREFIX  ?= /usr
SBINDIR ?= $(PREFIX)/sbin
MANDIR  ?= $(PREFIX)/share/man
LIBDIR  ?= $(PREFIX)/lib

all: wsdd2

nl_debug: CPPFLAGS+=-DMAIN
nl_debug: nl_debug.c; $(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

wsdd2: $(OBJFILES)
$(OBJFILES): $(HEADERS) Makefile

install: wsdd2
	install -d $(INSTALLDIR)$(SBINDIR)
	install wsdd2 $(INSTALLDIR)$(SBINDIR)

clean:
	rm -f wsdd2 nl_debug $(OBJFILES)
