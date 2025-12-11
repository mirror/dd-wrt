musl-fts-configure:
	cd musl-fts && libtoolize
	cd musl-fts && aclocal
	cd musl-fts && autoconf
	cd musl-fts && autoheader
	cd musl-fts && automake --add-missing
	cd musl-fts && ./configure \
		--host=$(ARCH)-linux \
		--prefix=/usr \
		--disable-shared \
		--enable-static \
		CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -std=gnu89 -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -std=gnu89 -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -std=gnu89 -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral -Wno-error=use-after-free -ffunction-sections -fdata-sections -Wl,--gc-sections"

musl-fts:
	$(MAKE) -C musl-fts

musl-fts-install:
	@true
