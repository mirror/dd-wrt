curl-install:
	$(MAKE) -C curl/build install DESTDIR=$(INSTALLDIR)/curl
	rm -f $(INSTALLDIR)/curl/usr/bin/curl-config
	rm -rf $(INSTALLDIR)/curl/usr/include
	rm -rf $(INSTALLDIR)/curl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/curl/usr/lib/*.a
	rm -f $(INSTALLDIR)/curl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/curl/usr/share
	mkdir -p $(INSTALLDIR)/curl/etc/ssl/certs
	cp $(TOP)/curl/build/lib/ca-bundle.crt $(INSTALLDIR)/curl/etc/ssl/certs/ca-certificates.crt

curl: openssl zlib
	$(MAKE) -C curl/build

curl-clean:
	$(MAKE) -C curl/build clean

curl-configure: zlib openssl
	-$(MAKE) -C openssl
	$(MAKE) -C zlib clean
	$(MAKE) -C zlib
	cd curl && autoreconf -fi
	mkdir -p curl/build
	mkdir -p curl/build_static
	cd curl/build && ../configure --disable-verbose \
	--disable-ntlm \
	--disable-debug \
	--disable-ares \
	--disable-manual \
	--without-nss \
	--without-librtmp \
	--without-libidn \
	--without-ca-path \
	--without-libpsl \
	--without-zstd \
	--with-ca-bundle=/etc/ssl/certs/ca-certificates.crt --with-openssl --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="-DNEED_PRINTF $(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib  -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/zlib -L$(SSLPATH) -lcrypto -lssl -ldl" \
	AR_FLAGS="\"cru $(LTOPLUGIN)\"" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	NM="$(ARCH)-linux-nm $(LTOPLUGIN)" \
	lt_cv_sys_global_symbol_pipe="sed -n -e 's/^.*[	 ]\\([ABCDGIRSTW][ABCDGIRSTW]*\\)[	 ][	 ]*\\([_A-Za-z][_A-Za-z0-9]*\\)\$$/\\1 \\2 \\2/p' | sed '/ __gnu_lto/d'" \
	lt_cv_sys_global_symbol_to_cdecl="sed -n -e 's/^T .* \\(.*\\)\$$/extern int \\1();/p' -e 's/^[ABCDGIRSTW][ABCDGIRSTW]* .* \\(.*\\)\$$/extern char \\1;/p'"

	$(MAKE) -C curl/build
	$(MAKE) -C curl/build ca-bundle
