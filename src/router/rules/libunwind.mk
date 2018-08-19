libunwind-configure:
	cd libunwind && ./configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--disable-documentation \
		--enable-minidebuginfo=no \
		--enable-fast-install=yes \
		--disable-tests \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -DNEED_PRINTF -D_GNU_SOURCE" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl"

libunwind: 
	$(MAKE) -C libunwind

libunwind-clean: 
	if test -e "libunwind/Makefile"; then $(MAKE) -C libunwind clean ; fi

libunwind-install: 
	$(MAKE) -C libunwind install DESTDIR=$(INSTALLDIR)/libunwind
	rm -rf $(INSTALLDIR)/libunwind/usr/man
	rm -rf $(INSTALLDIR)/libunwind/usr/include
	rm -f $(INSTALLDIR)/libunwind/usr/lib/*.a
	rm -f $(INSTALLDIR)/libunwind/usr/lib/*.la
	rm -rf $(INSTALLDIR)/libunwind/usr/lib/pkgconfig
	