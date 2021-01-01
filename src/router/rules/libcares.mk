
libcares:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC" \
	$(MAKE) -C libcares

libcares-install:
	$(MAKE) -C libcares install DESTDIR=$(INSTALLDIR)/libcares
	rm -rf $(INSTALLDIR)/libcares/include
	rm -f $(INSTALLDIR)/libcares/usr/lib/*.a
	rm -f $(INSTALLDIR)/libcares/usr/lib/*.la
	rm -rf $(INSTALLDIR)/libcares/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libcares/usr/lib/share

libcares-clean:
	$(MAKE) -C libcares clean

libcares-configure:
	cd libcares && ./configure ac_cv_host=$(ARCH)-uclibc-linux --prefix=/usr --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc"
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-section"
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC" \
	$(MAKE) -C libcares
