curl: openssl zlib
	$(MAKE) -C curl

curl-install:
	$(MAKE) -C curl install DESTDIR=$(INSTALLDIR)/curl
	rm -f $(INSTALLDIR)/curl/usr/bin/curl-config
	rm -rf $(INSTALLDIR)/curl/usr/include
	rm -rf $(INSTALLDIR)/curl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/curl/usr/lib/*.a
	rm -f $(INSTALLDIR)/curl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/curl/usr/share
	mkdir -p $(INSTALLDIR)/curl/etc/ssl
	cp $(TOP)/curl/lib/ca-bundle.crt $(INSTALLDIR)/curl/etc/ssl/

curl-clean:
	$(MAKE) -C curl clean

curl-configure: openssl zlib
	cd curl && ./buildconf && ./configure --with-ca-bundle=/etc/ssl/ca-bundle.crt --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib  -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypto -lssl -ldl" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	NM="$(ARCH)-linux-nm $(LTOPLUGIN)" \
	lt_cv_sys_global_symbol_pipe="sed -n -e 's/^.*[	 ]\\([ABCDGIRSTW][ABCDGIRSTW]*\\)[	 ][	 ]*\\([_A-Za-z][_A-Za-z0-9]*\\)\$$/\\1 \\2 \\2/p' | sed '/ __gnu_lto/d'" \
	lt_cv_sys_global_symbol_to_cdecl="sed -n -e 's/^T .* \\(.*\\)\$$/extern int \\1();/p' -e 's/^[ABCDGIRSTW][ABCDGIRSTW]* .* \\(.*\\)\$$/extern char \\1;/p'"
	$(MAKE) -C curl
	$(MAKE) -C curl ca-bundle
