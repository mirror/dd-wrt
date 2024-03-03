wolfssl-configure:
	cd wolfssl && ./autogen.sh
	mkdir -p wolfssl/minimal
	mkdir -p wolfssl/standard
	cd wolfssl/standard && ../configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux --disable-examples --enable-opensslextra --enable-opensslall --enable-shared --enable-fastmath --disable-static -disable-errorstrings --enable-lowresource --disable-oldtls --enable-aesgcm --enable-poly1305 --enable-chacha --enable-ecc --disable-sslv3 --enable-des3 --enable-md4 --enable-stunnel --enable-tls13 --enable-session-ticket --enable-wpas --enable-cmac CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd wolfssl/minimal && ../configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux --enable-opensslextra --disable-shared --enable-fastmath --enable-static -disable-errorstrings --disable-oldtls --disable-sha --enable-lowresource --disable-md5 --disable-rc4 --disable-poly1305 --disable-chacha --enable-ecc --disable-sslv3 --disable-tls13 --disable-des3 --enable-md4 --enable-stunnel --enable-session-ticket --enable-cmac CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	$(MAKE) -C wolfssl/minimal
	$(MAKE) -C wolfssl/standard

wolfssl:
ifeq ($(CONFIG_WOLFSSLMIN),y)
	$(MAKE) -C wolfssl/minimal
else
	$(MAKE) -C wolfssl/standard
endif
	-rm wolfssl/wolfssl/options.h

wolfssl-clean:
	-make -C wolfssl/minimal clean
	-make -C wolfssl/standard clean
	@true

wolfssl-install:
ifneq ($(CONFIG_WOLFSSLMIN),y)
	$(MAKE) -C wolfssl/standard install DESTDIR=$(INSTALLDIR)/wolfssl
	rm -rf $(INSTALLDIR)/wolfssl/usr/bin
	rm -rf $(INSTALLDIR)/wolfssl/usr/include
	rm -rf $(INSTALLDIR)/wolfssl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/wolfssl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/wolfssl/usr/share
else
	@true
endif
