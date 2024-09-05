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
		CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -DNEED_PRINTF -D_GNU_SOURCE -mno-outline-atomics" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH)"
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/libunwind/config/ltmain.sh
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/libunwind/libtool

libunwind: 
ifneq ($(ARCH),powerpc)
ifneq ($(ARCH),i386)
	$(MAKE) -C libunwind
endif
endif

libunwind-clean: 
ifneq ($(ARCH),i386)
ifneq ($(ARCH),powerpc)
	if test -e "libunwind/Makefile"; then $(MAKE) -C libunwind clean ; fi
endif
endif

libunwind-install: 
ifneq ($(ARCH),powerpc)
ifneq ($(ARCH),i386)
	$(MAKE) -C libunwind install DESTDIR=$(INSTALLDIR)/libunwind
	rm -rf $(INSTALLDIR)/libunwind/usr/man
	rm -rf $(INSTALLDIR)/libunwind/usr/include
	rm -f $(INSTALLDIR)/libunwind/usr/lib/*.a
	rm -f $(INSTALLDIR)/libunwind/usr/lib/*.la
	rm -rf $(INSTALLDIR)/libunwind/usr/lib/pkgconfig
endif
endif