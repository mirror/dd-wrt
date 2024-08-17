curl-static-clean:
	$(MAKE) -C bearssl
	$(MAKE) -C curl/build_static clean


curl-static: bearssl
	$(MAKE) -C curl/build_static

curl-static-install:
	@true

curl-install:
	$(MAKE) -C curl/build install DESTDIR=$(INSTALLDIR)/curl
	rm -f $(INSTALLDIR)/curl/usr/bin/curl-config
	rm -rf $(INSTALLDIR)/curl/usr/include
	rm -rf $(INSTALLDIR)/curl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/curl/usr/lib/*.a
	rm -f $(INSTALLDIR)/curl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/curl/usr/share
	mkdir -p $(INSTALLDIR)/curl/etc/ssl
	cp $(TOP)/curl/build/lib/ca-bundle.crt $(INSTALLDIR)/curl/etc/ssl/

curl: openssl zlib
	$(MAKE) -C curl/build

curl-clean:
	$(MAKE) -C curl/build clean

curl-configure: openssl zlib
	$(MAKE) -C zlib clean
	$(MAKE) -C zlib
	$(MAKE) -C bearssl clean
	$(MAKE) -C bearssl
	cd curl && ./buildconf
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
	--with-ca-bundle=/etc/ssl/ca-bundle.crt --with-openssl --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="-DNEED_PRINTF $(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib  -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/zlib -L$(SSLPATH) -lcrypto -lssl -ldl" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	NM="$(ARCH)-linux-nm $(LTOPLUGIN)" \
	lt_cv_sys_global_symbol_pipe="sed -n -e 's/^.*[	 ]\\([ABCDGIRSTW][ABCDGIRSTW]*\\)[	 ][	 ]*\\([_A-Za-z][_A-Za-z0-9]*\\)\$$/\\1 \\2 \\2/p' | sed '/ __gnu_lto/d'" \
	lt_cv_sys_global_symbol_to_cdecl="sed -n -e 's/^T .* \\(.*\\)\$$/extern int \\1();/p' -e 's/^[ABCDGIRSTW][ABCDGIRSTW]* .* \\(.*\\)\$$/extern char \\1;/p'"

	cd curl/build_static && ../configure \
	--disable-pthreads \
	--disable-threaded-resolver \
	--disable-progress-meter \
	--disable-verbose \
	--disable-http-auth \
	--disable-mime \
	--disable-libcurl-option \
	--disable-largefile \
	--disable-netrc \
	--disable-socketpair \
	--disable-cookies \
	--disable-dateparse \
	--disable-dnsshuffle \
	--disable-option-checking \
	--disable-get-easy-options \
	--disable-doh \
	--disable-crypto-auth \
	--disable-openssl-auto-load-config \
	--disable-ftp \
	--disable-gopher \
	--disable-ldap \
	--disable-ldaps \
	--disable-rtsp \
	--disable-proxy \
	--disable-dict \
	--disable-hsts \
	--disable-file \
	--disable-alt-svc \
	--disable-telnet \
	--disable-tftp \
	--disable-pop3 \
	--disable-imap \
	--disable-smb \
	--disable-smtp \
	--disable-mqtt \
	--disable-ech \
	--disable-curldebug \
	--disable-shared \
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
	--without-zlib \
	--without-ngtcp2 \
	--without-nghttp3 \
	--without-quiche \
	--without-msh3 \
	--disable-websockets \
	--disable-headers-api \
	--enable-static \
	--with-ca-bundle=/etc/ssl/ca-bundle.crt \
	--without-openssl \
	--with-bearssl \
	--prefix=/usr \
	ac_cv_host=$(ARCH)-uclibc-linux \
	--libdir=/usr/lib \
	--target=$(ARCH)-linux \
	--host=$(ARCH) \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/bearssl/inc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/bearssl/build -lbearssl -ldl" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	NM="$(ARCH)-linux-nm $(LTOPLUGIN)" \
	lt_cv_sys_global_symbol_pipe="sed -n -e 's/^.*[	 ]\\([ABCDGIRSTW][ABCDGIRSTW]*\\)[	 ][	 ]*\\([_A-Za-z][_A-Za-z0-9]*\\)\$$/\\1 \\2 \\2/p' | sed '/ __gnu_lto/d'" \
	lt_cv_sys_global_symbol_to_cdecl="sed -n -e 's/^T .* \\(.*\\)\$$/extern int \\1();/p' -e 's/^[ABCDGIRSTW][ABCDGIRSTW]* .* \\(.*\\)\$$/extern char \\1;/p'"

	$(MAKE) -C curl/build
	$(MAKE) -C curl/build ca-bundle
	$(MAKE) -C curl/build_static
	$(MAKE) -C curl/build_static ca-bundle
