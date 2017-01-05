curl:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypo -lssl -ldl" \
	$(MAKE) -C curl

curl-install:
	$(MAKE) -C curl install DESTDIR=$(INSTALLDIR)/curl
	rm -f $(INSTALLDIR)/curl/usr/bin/curl-config
	rm -rf $(INSTALLDIR)/curl/usr/include
	rm -rf $(INSTALLDIR)/curl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/curl/usr/lib/*.a
	rm -f $(INSTALLDIR)/curl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/curl/usr/share

curl-clean:
	$(MAKE) -C curl clean

curl-configure:
	cd curl && aclocal && automake && ./configure --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib  -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypto -lssl -ldl" 
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypo -lssl -ldl" \
	$(MAKE) -C curl
