include $(TOP)/.config

WRT_BUILDROOT=/opt/

#CC=mipsel-linux-uclibc-gcc
#STRIP=mipsel-linux-uclibc-strip


ifeq ($(CONFIG_MSSID),y)
CFLAGS=$(COPTS) -I. -I../shared -I../services -I../../include.v24
else
CFLAGS=$(COPTS) -Wall -I. -I../shared -I../services -I../../include.v23
endif

ifeq ($(CONFIG_MADWIFI),y)
  CFLAGS += -DHAVE_MADWIFI -I../madwifi/madwifi.dev -include ../madwifi/madwifi.dev/include/compat.h -DHEADERS_KERNEL
endif

all: wrt-radauth radius-client macupd

wrt-radauth: wrt-radauth.o md5.o radius.o
	$(CC) $(LDFLAGS) -L../shared -L../nvram -lshared -lnvram $^ -o $@

radius-client: radius-client.o md5.o radius.o

macupd: macupd.o

wrt-radauth.o: wrt-radauth.c radius.h

radius-client.o: radius-client.c radius.h

md5.o: md5.c md5.h

radius.o: radius.c radius.h


strip: all
	$(STRIP) wrt-radauth radius-client macupd

clean:
	rm -f wrt-radauth md5.o wrt-radauth.o radius.o radius-client.o radius-client macupd macupd.o

dist:
	cd ..; rm -f wrt-radauth.tar.gz; tar -c -v --exclude=wrt-radauth/CVS -z -f wrt-radauth.tar.gz wrt-radauth/

.PHONY: all clean strip dist
