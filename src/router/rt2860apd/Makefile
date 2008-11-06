PLATFORM = RT2880

ifndef CFLAGS
CFLAGS = -O2 -Wall -g
endif

# If you want to support multiple radius server for multiple bssid, add following line
CFLAGS +=  -DMULTIPLE_RADIUS -DNEED_PRINTF

# If you want to debug daemon, add following line
CFLAGS +=  -DDBG 

OBJS =	rt2860apd.o eloop.o eapol_sm.o radius.o md5.o  \
	config.o ieee802_1x.o  \
	sta_info.o   radius_client.o

all: rt2860apd

rt2860apd: $(OBJS)
	$(CC) -o rt2860apd $(OBJS)

clean:
	rm -f core *~ *.o rt2860apd *.d

install:
	install -d $(INSTALLDIR)/usr/sbin
	install rt2860apd $(INSTALLDIR)/usr/sbin/rt2860apd

