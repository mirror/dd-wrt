
tor-configure: openssl xz zstd libevent zlib
	cd tor && libtoolize -ci --force 
	cd tor && aclocal
	cd tor && automake --add-missing
	cd tor && autoreconf -fi 
	cd tor && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --disable-systemd --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--disable-asciidoc \
	--enable-lzma \
	--enable-zstd \
	--disable-unittests \
	--disable-tool-name-check \
	--disable-gcc-hardening \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG  -std=gnu99 -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(SSLPATH)/include -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/xz/src/liblzma/api -I$(TOP)/zstd/lib" \
	CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -std=gnu99 -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(SSLPATH)/include -I$(TOP)/libevent  -I$(TOP)/libevent/include -I$(TOP)/xz/src/liblzma/api -I$(TOP)/zstd/lib" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -std=gnu99  -L$(TOP)/zlib   -L$(SSLPATH) -L$(TOP)/libevent/.libs -L$(TOP)/xz/src/liblzma/.libs -L$(TOP)/zstd/lib" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	ARFLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	ZSTD_CFLAGS="-I$(TOP)/zstd/lib" \
	ZSTD_LIBS="-I$(TOP)/zstd/lib -lzstd"

tor: openssl libevent zlib
	install -D tor/config/tor.webservices httpd/ej_temp/
	make -C tor ARFLAGS="cru $(LTOPLUGIN)"  all

tor-clean:
	make -C tor clean

tor-install:
	make -C tor DESTDIR=$(INSTALLDIR)/tor install

	mkdir -p $(INSTALLDIR)/tor/etc/config
	install -D tor/config/tor.webservices $(INSTALLDIR)/tor/etc/config
	install -D tor/config/tor.nvramconfig $(INSTALLDIR)/tor/etc/config