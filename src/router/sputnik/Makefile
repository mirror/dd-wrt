CFLAGS += -Os -Wall -fpic

ARCH = powerpc
#CC = /opt/staging_dir_armeb/bin/armeb-linux-uclibc-gcc
CC = $(ARCH)-linux-uclibc-gcc

CFLAGS += -I iksemel
CFLAGS += -I zlib
CFLAGS += -I ../../include.v23 -I../shared
CFLAGS += -DIPTABLES=\"/usr/sbin/iptables\"

# LDFLAGS += -L$(INSTALLDIR)/nvram/usr/lib -lnvram -lshared
LDFLAGS = -L../shared -lshared  -L../nvram -lnvram -lcrypt

OBJS := src/apd.o
IOBJS += iksemel/base64.o
IOBJS += iksemel/dom.o
IOBJS += iksemel/filter.o
IOBJS += iksemel/iks.o
IOBJS += iksemel/ikstack.o
IOBJS += iksemel/io-posix.o
IOBJS += iksemel/jabber.o
IOBJS += iksemel/md5.o
IOBJS += iksemel/sax.o
IOBJS += iksemel/sha.o
IOBJS += iksemel/stream.o
IOBJS += iksemel/utility.o

OBJS += zlib/adler32.o
OBJS += zlib/compress.o
OBJS += zlib/crc32.o
OBJS += zlib/deflate.o
OBJS += zlib/gzio.o
OBJS += zlib/infback.o
OBJS += zlib/inffast.o
OBJS += zlib/inflate.o
OBJS += zlib/inftrees.o
OBJS += zlib/trees.o
OBJS += zlib/uncompr.o
OBJS += zlib/zutil.o

OBJS += src/base64.o
OBJS += src/blowfish.o
OBJS += src/brcm.o
OBJS += src/cbc.o
OBJS += src/control.o
OBJS += src/diag.o
OBJS += src/duette.o
OBJS += src/firewall.o
OBJS += src/hexstring.o
OBJS += src/httpd.o
OBJS += src/jauth.o
OBJS += src/keys.o
OBJS += src/license.o
OBJS += src/log.o
OBJS += src/lsk.o
OBJS += src/md5.o
OBJS += src/md5_util.o
OBJS += src/pf.o
OBJS += src/pppoe.o
OBJS += src/status.o
OBJS += src/usage.o

all: libiksemel.so sputnik sputnik.static

clean:
	rm -f $(OBJS) apd
	rm -f $(IOBJS) apd

install: all
	install -d $(INSTALLDIR)/sbin
	install apd $(INSTALLDIR)/sbin
	$(STRIP) $(INSTALLDIR)/sbin/apd

libiksemel.so: $(IOBJS)
	$(CC) -shared -o $(ARCH)/$@ $^ $(LDFLAGS) -fpic
	
sputnik: $(OBJS) 
	$(CC) -o $(ARCH)/$@ $^ $(LDFLAGS) -L$(ARCH) -liksemel

sputnik.static: $(OBJS) $(IOBJS)
	$(CC) -o $(ARCH)/$@ $^ $(LDFLAGS) -L$(ARCH) 

$(OBJS): $(CY_DEPS)
