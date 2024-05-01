ipset-configure:
	cd ipset && ./autogen.sh
	cd ipset && ./configure libmnl_CFLAGS="-I$(TOP)/libmnl/include" libmnl_LIBS="-L$(TOP)/libmnl/src/.libs -lmnl" --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static --with-kmod=no --libdir=/usr/lib \
		CC="$(CC)" \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

ipset: libmnl
	$(MAKE) -C ipset

ipset-clean:
	$(MAKE) -C ipset clean

ipset-install:
	make -C ipset install-exec DESTDIR=$(INSTALLDIR)/ipset
	rm -rf $(INSTALLDIR)/ipset/usr/lib
