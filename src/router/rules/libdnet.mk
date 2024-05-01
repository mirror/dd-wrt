ifeq ($(CONFIG_IPV6),y)
libdnet-configure:
	cd libdnet && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=$(TOP)/libdnet/src/.libs/ \
		--disable-shared \
		--enable-static \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -DNEED_PRINTF -I$(TOP)/iptables-new/include/libipq/" LDFLAGS="-L$(TOP)/iptables-new/libipq/.libs"
else
libdnet-configure:
	cd libdnet && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=$(TOP)/libdnet/src/.libs/ \
		--disable-shared \
		--enable-static \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"
endif

libdnet:
	$(MAKE) -C libdnet CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -DNEED_PRINTF"

libdnet-install:
	@true
#	install -D libdnet/src/.libs/libdnet.1 $(INSTALLDIR)/libdnet/usr/lib/libdnet.1
