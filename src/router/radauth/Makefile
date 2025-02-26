include $(TOP)/.config

WRT_BUILDROOT=/opt/

#CC=mipsel-linux-uclibc-gcc
#STRIP=mipsel-linux-uclibc-strip

ifeq ($(CONFIG_80211AC),y)
CFLAGS	= -I. -I$(LINUXDIR)/brcm/$(ARCH)/include  -I$(LINUXDIR)/brcm/$(ARCH)/common/include -Wall -I$(LINUXDIR)/brcm/$(ARCH)/shared/bcmwifi/include

endif

ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CFLAGS+=$(COPTS) -fomit-frame-pointer -I. -I../shared -I../services -I../../include.bcm -DNEW_RSSI
else
CFLAGS+=$(COPTS) -fomit-frame-pointer -I. -I../shared -I../services -I../../include.v24 -DNEW_RSSI
endif
else
CFLAGS+=$(COPTS) -fomit-frame-pointer -Wall -I. -I../shared -I../services -I../../include.v23
endif

ifeq ($(CONFIG_MADWIFI),y)
  CFLAGS += -DHAVE_MADWIFI -I../madwifi/ -include ../madwifi/include/compat.h -DHEADERS_KERNEL
endif
ifeq ($(CONFIG_ATH9K),y)
  LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
  CFLAGS += -DHAVE_MADWIFI -DHAVE_MADWIFI_MIMO -I../madwifi/ -include ../madwifi/include/compat.h -DHEADERS_KERNEL
endif
ifeq ($(CONFIG_RT2880),y)
  CFLAGS += -DHAVE_RT2880 
endif
ifeq ($(CONFIG_RT61),y)
  CFLAGS += -DHAVE_RT2880 
endif

CFLAGS += $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -fpic -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(LINUXDIR)/brcm/shared/bcmwifi/include
LDFLAGS += $(COPTS) -L -ffunction-sections -fdata-sections -Wl,--gc-sections

#ifeq ($(CONFIG_ATH9K),y)
#LDFLAGS += -L$(TOP)/libnl-tiny -lnl-tiny
#endif

all: wrt-radauth radius-client macupd

wrt-radauth: wrt-radauth.o md5.o radius.o
	$(CC) $(LDFLAGS) -L../libutils -L../shared -L../nvram -lutils -lshutils -lnvram -lwireless $^ -o $@

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
