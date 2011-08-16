libdnet-configure:
	cd libdnet && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


libdnet:
	$(MAKE) -C libdnet CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF"

libdnet-install:
	@true
