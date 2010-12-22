include $(TOP)/.config
CFLAGS += $(COPTS)

#ARCH = mipsel
#CC = /opt/staging_dir_armeb/bin/armeb-linux-uclibc-gcc
CC = $(ARCH)-linux-uclibc-gcc

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif


CFLAGS += -I iksemel -fPIC
CFLAGS += -I zlib 
CFLAGS += -I ../../include.v23 -I../shared
CFLAGS += -DIPTABLES=\"/usr/sbin/iptables\"
ifeq ($(CONFIG_MAKSAT),y)
CFLAGS += -DAPD_PLATFORM_WRT54GS -DAPD_PLATFORM=\"maksat\" -DAPD_PLATFORM_DDWRT -DDEFAULT_MGMTJID=\"sputnik@wifi.sputnik.com\" -DAPD_SNE_MGMTJID=\"express@express.sputnik.com\" -DAPD_VERSION=\"3.3.6.6.1\"
else
CFLAGS += -DAPD_PLATFORM_WRT54GS -DAPD_PLATFORM=\"dd-wrt\" -DAPD_PLATFORM_DDWRT -DDEFAULT_MGMTJID=\"sputnik@wifi.sputnik.com\" -DAPD_SNE_MGMTJID=\"express@express.sputnik.com\" -DAPD_VERSION=\"3.3.6.6.1\"
endif
#CFLAGS += -DAPD_PLATFORM_WRT54GS -DAPD_PLATFORM=\"dd-wrt\" -DAPD_PLATFORM_DDWRT -DDEFAULT_MGMTJID=\"sputnik@wifi.sputnik.com\" -DAPD_SNE_MGMTJID=\"express@express.sputnik.com\" -DAPD_VERSION=\"3.3.6.6\"

# LDFLAGS += -L$(INSTALLDIR)/nvram/usr/lib -lnvram -lshared
LDFLAGS =-L../libutils -lwireless -lutils -L../nvram -lnvram -lcrypt
ifeq ($(CONFIG_MADWIFI),y)
LDFLAGS += -lm
endif

ifeq ($(CONFIG_ATH9K),y)
LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
endif

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
ifeq ($(CONFIG_BCMMODERN),y)
	$(CC) -fPIC -shared -o brcm26/$@ $^ $(LDFLAGS) 
else
	$(CC) -fPIC -shared -o $(ARCH)/$@ $^ $(LDFLAGS) 
endif
	
sputnik: $(OBJS) 
ifeq ($(CONFIG_BCMMODERN),y)
	$(CC) -o brcm26/$@ $^ $(LDFLAGS) -L$(ARCH) -liksemel
else
	$(CC) -o $(ARCH)/$@ $^ $(LDFLAGS) -L$(ARCH) -liksemel
endif

sputnik.static: $(OBJS) $(IOBJS)
ifeq ($(CONFIG_BCMMODERN),y)
	$(CC) -o brcm26/$@ $^ $(LDFLAGS) -L$(ARCH) 
else
	$(CC) -o $(ARCH)/$@ $^ $(LDFLAGS) -L$(ARCH) 
endif

$(OBJS): $(CY_DEPS)
