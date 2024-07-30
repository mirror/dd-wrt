# Makefile for hdparm

# DESTDIR is for non root installs (eg packages, NFS) only!
DESTDIR = $(INSTALLDIR)

binprefix = 
manprefix = /usr
exec_prefix = $(binprefix)/
sbindir = $(exec_prefix)sbin
mandir = $(manprefix)/share/man
oldmandir = $(manprefix)/man

CC ?= gcc
STRIP ?= strip

CFLAGS := $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF  -W -Wall -Wbad-function-cast -Wcast-align -Wpointer-arith -Wcast-qual -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -fkeep-inline-functions -Wwrite-strings -Waggregate-return -Wnested-externs -Wtrigraphs

LDFLAGS = -s
#LDFLAGS = -s -static
INSTALL = install
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_DIR = $(INSTALL) -m 755 -d
INSTALL_PROGRAM = $(INSTALL)

OBJS = hdparm.o identify.o sgio.o sysfs.o geom.o fallocate.o fibmap.o fwdownload.o dvdspeed.o wdidle3.o apt.o

all:
	$(MAKE) -j4 hdparm

hdparm: hdparm.h sgio.h $(OBJS)
	$(CC) $(LDFLAGS) $(COPTS) -o hdparm $(OBJS)
	$(STRIP) hdparm

hdparm.o:	hdparm.h sgio.h

identify.o:	hdparm.h

dvdspeed.o:     dvdspeed.c

sgio.o: sgio.c sgio.h hdparm.h

fwdownload.o: fwdownload.c sgio.h hdparm.h

apt.o:		apt.c

install: all hdparm.8
	if [ ! -z $(DESTDIR) ]; then $(INSTALL_DIR) $(DESTDIR) ; fi
	if [ ! -z $(DESTDIR)$(sbindir) ]; then $(INSTALL_DIR) $(DESTDIR)$(sbindir) ; fi
	if [ -f $(DESTDIR)$(sbindir)/hdparm ]; then rm -f $(DESTDIR)$(sbindir)/hdparm ; fi
	$(INSTALL_PROGRAM) -D hdparm $(DESTDIR)$(sbindir)/hdparm

clean:
	-rm -f hdparm $(OBJS) core 2>/dev/null

