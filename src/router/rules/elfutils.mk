elfutils: zlib argp-standalone musl-fts
	$(MAKE) -C elfutils

elfutils-install:
	$(MAKE) -C elfutils install DESTDIR=$(INSTALLDIR)/elfutils
	rm -rf $(INSTALLDIR)/elfutils/usr/share
	rm -rf $(INSTALLDIR)/elfutils/usr/include
	rm -rf $(INSTALLDIR)/elfutils/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/elfutils/usr/lib/*.a

elfutils-configure: zlib argp-standalone musl-fts
	cd elfutils && autoreconf
	cd elfutils && ./configure \
		ac_cv_search__obstack_free=yes \
		--disable-nls \
		--host=$(ARCH)-linux \
		--prefix=/usr \
		--libdir=/usr/lib \
		--disable-debuginfod \
		--disable-static \
		--enable-shared \
		--disable-libdebuginfod \
		--without-bzlib \
		--without-lzma \
		--without-zstd \
		CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/musl-fts -I$(TOP)/argp-standalone -I$(TOP)/zlib -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/musl-fts -I$(TOP)/argp-standalone -I$(TOP)/zlib -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/argp-standalone -L$(TOP)/musl-fts/.libs -lfts -L$(TOP)/zlib -lz  -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -ffunction-sections -fdata-sections -Wl,--gc-sections"

elfutils-clean:
	$(MAKE) -C elfutils clean

