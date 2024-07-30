CFLAGS += $(MIPS16_OPT) $(COPTS) $(LTO) -I$(TOP)/ipeth/libimobiledevice/include -I$(TOP)/ipeth/libplist/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections 
LDLIBS += $(TOP)/ipeth/libimobiledevice/src/.libs/libimobiledevice-1.0.a -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0 $(TOP)/ipeth/libusbmuxd/src/.libs/libusbmuxd-2.0.a -lm -lpthread -ldl  -ffunction-sections -fdata-sections -Wl,--gc-sections 

all: ipheth-pair ipheth-loop

ipheth-pair: ipheth-pair.o

ipheth-loop: ipheth-loop.o
	$(CC) $(COPTS) -o ipheth-loop  -fdata-sections -Wl,--gc-sections ipheth-loop.o

#install:
#	install -m 0755 ipheth-pair /lib/udev/
#	install -m 0644 90-iphone-tether.rules /etc/udev/rules.d
#	/sbin/udevadm control --reload-rules

clean:
	rm -f *.o ipheth-pair ipheth-loop
