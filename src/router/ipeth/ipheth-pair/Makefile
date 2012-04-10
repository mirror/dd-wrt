CFLAGS += $(COPTS) -I$(TOP)/ipeth/libimobiledevice/include -I$(TOP)/ipeth/libplist/include -DNEED_PRINTF
LDLIBS += -L$(TOP)/ipeth/libimobiledevice/src/.libs -limobiledevice -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/ipeth/libplist/src/ -lplist -L$(TOP)/ipeth/libusbmuxd/libusbmuxd/ -lusbmuxd

all: ipheth-pair

ipheth-pair: ipheth-pair.o

install:
	install -m 0755 ipheth-pair /lib/udev/
	install -m 0644 90-iphone-tether.rules /etc/udev/rules.d
	/sbin/udevadm control --reload-rules

clean:
	rm -f *.o ipheth-pair
