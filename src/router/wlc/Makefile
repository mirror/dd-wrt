ifeq ($(CONFIG_MSSID),y)
CFLAGS	= -I$(SRCBASE)/include.v24 -Wall
else
CFLAGS	= -I$(SRCBASE)/include.v23 -Wall 
endif
LDFLAGS = -L../libutils -L../nvram -lutils -lnvram


all: wlc
clean:
	rm -f *.o wlc

wlc: wlc.o ioctl.o
%.o: %.c
	$(CC) $(COPTS) $(CFLAGS)-lshared -I$(TOP)/shared -c -o $@ $<

install:
	install -D wlc $(INSTALLDIR)/usr/sbin/wl

.PHONY: all clean
