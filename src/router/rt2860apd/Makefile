-include ../.config


PLATFORM = RT2880


# If you want to support multiple radius server for multiple bssid, add following line
CFLAGS +=  -DMULTIPLE_RADIUS -DNEED_PRINTF

CFLAGS += $(COPTS) -DHAVE_AQOS 

CFLAGS += $(MIPS16_OPT)
LDFLAGS += $(MIPS16_OPT)
LDFLAGS += -L$(TOP)/libutils -L$(TOP)/nvram -lshutils -lutils -lqos -lnvram $(ADDLIBS)


# If you want to debug daemon, add following line
#CFLAGS +=  -DDBG 

OBJS =	rt2860apd.o eloop.o eapol_sm.o radius.o md5.o  \
	config.o ieee802_1x.o  \
	sta_info.o   radius_client.o

all: rt2860apd

rt2860apd: $(OBJS)
	$(CC) -o rt2860apd $(OBJS) $(LDFLAGS)

clean:
	rm -f core *~ *.o rt2860apd *.d

install:
	install -d $(INSTALLDIR)/usr/sbin
	install rt2860apd $(INSTALLDIR)/usr/sbin/rt2860apd

