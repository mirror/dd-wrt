POEOBJ = main.o dialect_bcm.o dialect_rtl.o

CFLAGS += -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -I$(TOP) -I$(TOP)/ubus -I$(TOP)/shared
LDFLAGS += -L$(TOP)/ubus -L$(TOP)/libubox -L$(TOP)/nvram -L$(TOP)/libutils -lubox -lubus -lnvram -lshutils -lutils
all: realtek-poe

realtek-poe: $(POEOBJ)
	$(CC) $^ $(LDFLAGS) $(CFLAGS) $(LDLTO) $(LDLIBS) -o $@

install: all
	mkdir -p $(INSTALLDIR)/usr/sbin
	install -m 0755 realtek-poe $(INSTALLDIR)/usr/sbin

clean:
	rm -f $(BROBJ) realtek-poe
