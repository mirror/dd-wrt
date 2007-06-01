#! make -f
#
# $Id: Makefile,v 1.47 2005/05/29 15:32:35 jordan Exp $
#

VERSION  = 0.9.7

#CC = mipsel-linux-uclibc-gcc
# UNWANTED = -DDISCARD_INFO -DDISCARD_CMD_SOCKET
NETBIOS = -DDROP_NETBIOS # -DALLOW_WINS
# PROF  = -pg -fprofile-arcs -D_DEBUG
# DEBUG = -g -D_DEBUG

OPTIM =  $(COPTS)

# install service
PROGNAME   = shatd
SBINDIR    = /usr/sbin
INSTALLDIR = $(TARGET)$(SBINDIR)
INSTALLPRG = $(INSTALLDIR)/$(PROGNAME)

# compilation settings
MAINSRC = shatd.c
SOURCES = util.c do.c pool.c device.c lookup.c arp.c ip2ether.c \
          cleanup.c ctrl.c comhandler.c
HEADERS = $(SOURCES:.c=.h)
OBJECTS = $(SOURCES:.c=.o) $(MAINSRC:.c=.o)

CFLAGS  = -Wall $(DEBUG) $(OPTIM) $(UNWANTED) $(EXTRA)
CFLAGS += -DVERSION=\"$(VERSION)\" $(NETBIOS) $(PROF)

LDFLAGS = $(DEBUG) $(PROF)

PACGES  = libc6-dev

default: all

prepare: prepare.stamp
prepare.stamp:
	apt-get install $(PACGES)

all depend:: depends.mak
depends.mak: $(MAINSRC) $(SOURCES)
	$(CC) $(CFLAGS) -MM $(MAINSRC) $(SOURCES) >$@

all:: shatd shatc

shatc: shatc.o util.o
	$(CC) $(CFLAGS) -o $@ shatc.o util.o

shatd: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

include depends.mak

install: shatd
	test -d $(INSTALLDIR) || install -d -m 555 $(INSTALLDIR)
	rm -f     $(INSTALLPRG)
	cp  shatd $(INSTALLPRG)
	-strip    $(INSTALLPRG)
	chown   0 $(INSTALLPRG)
	chmod 500 $(INSTALLPRG)

uninstall:
	rm -f $(INSTALLPRG)

distclean clean empty::
	rm -f *.o core *~ depends.mak

distclean::
	-dpkg --purge $(PACGES)

distclean empty::
	rm -f shatd shatc prepare.stamp

# End
