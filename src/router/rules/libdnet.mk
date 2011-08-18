libdnet-configure:
	cd libdnet && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=$(TOP)/libdnet/src/.libs/ \
		--disable-shared \
		--enable-static \
		CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


libdnet:
	$(MAKE) -C libdnet CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF"

libdnet-install:
	@true
#	install -D libdnet/src/.libs/libdnet.1 $(INSTALLDIR)/libdnet/usr/lib/libdnet.1
