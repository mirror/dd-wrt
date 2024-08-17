aircrack-ng: pcre zlib
	$(MAKE) -C aircrack-ng

aircrack-ng-install:
	$(MAKE) -C aircrack-ng DESTDIR=$(INSTALLDIR)/aircrack-ng install
	-cp $(TOP)/aircrack-ng/scan $(INSTALLDIR)/aircrack-ng/usr/sbin
	-cp $(TOP)/aircrack-ng/scantidy $(INSTALLDIR)/aircrack-ng/usr/sbin
	rm -rf $(INSTALLDIR)/aircrack-ng/usr/share
	rm -rf $(INSTALLDIR)/aircrack-ng/usr/include
	-rm -f $(INSTALLDIR)/aircrack-ng/usr/lib/*.la

aircrack-ng-configure: pcre zlib
	cd aircrack-ng && libtoolize --force --copy --automake
	cd aircrack-ng && aclocal -I build/m4/stubs -I build/m4 ${ACLOCAL_FLAGS:-}
	cd aircrack-ng && autoconf
	cd aircrack-ng && autoheader
	cd aircrack-ng && automake --gnu --add-missing --force --copy -Wno-portability -Wno-portability
	cd aircrack-ng && ./configure --host=$(ARCH)-linux \
	--with-openssl="$(SSLPATH)" \
        --prefix=/usr \
        --disable-libnl \
        --libdir=/usr/lib \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	PCRE_CFLAGS="-I$(TOP)/pcre" \
	PCRE_LIBS="-L$(TOP)/pcre/.libs -lpcre" \
        OPENSSL_LDFLAGS="-L$(SSLPATH)" \
        OPENSSL_LIBS="-lssl -lcrypto" \
        CXXFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -DNEED_PRINTF -std=gnu99  -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE" \
        CFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -DNEED_PRINTF -std=gnu99  -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE" \
        LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(SSLPATH) -ffunction-sections -fdata-sections -Wl,--gc-sections"

aircrack-ng-clean:
	$(MAKE) -C aircrack-ng clean

