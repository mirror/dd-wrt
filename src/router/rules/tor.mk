miniupnpc:
	make -C miniupnpc OS="Linux" OS_STRING="DD-WRT" PREFIX="/usr" CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC"

miniupnpc-clean:
	make -C miniupnpc clean

miniupnpc-configure:
	make -C miniupnpc

miniupnpc-install:
	make -C miniupnpc install INSTALLPREFIX=$(INSTALLDIR)/miniupnpc/usr 
	rm -rf $(INSTALLDIR)/miniupnpc/usr/bin
	rm -rf $(INSTALLDIR)/miniupnpc/usr/include
	rm -rf $(INSTALLDIR)/miniupnpc/usr/share
	rm -f $(INSTALLDIR)/miniupnpc/usr/lib/*.a

tor-configure: openssl xz zstd libevent
	cd tor && libtoolize -ci --force 
	cd tor && autoreconf -fi 
	cd tor && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --disable-systemd --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--disable-asciidoc \
	--enable-lzma \
	--enable-zstd \
	--disable-unittests \
	--disable-tool-name-check \
	--disable-gcc-hardening \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -std=gnu99 -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(TOP)/openssl/include -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/xz/src/liblzma/api -I$(TOP)/zstd/lib" \
	CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -std=gnu99 -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(TOP)/openssl/include -I$(TOP)/libevent  -I$(TOP)/libevent/include -I$(TOP)/xz/src/liblzma/api -I$(TOP)/zstd/lib" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -std=gnu99  -L$(TOP)/zlib   -L$(TOP)/openssl -L$(TOP)/libevent/.libs -L$(TOP)/xz/src/liblzma/.libs -L$(TOP)/zstd/lib" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	ARFLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	ZSTD_CFLAGS="-I$(TOP)/zstd/lib" \
	ZSTD_LIBS="-I$(TOP)/zstd/lib -lzstd"

tor: openssl libevent
	make -C tor ARFLAGS="cru $(LTOPLUGIN)"  all

tor-clean:
	make -C tor clean

tor-install:
	make -C tor DESTDIR=$(INSTALLDIR)/tor install

	mkdir -p $(INSTALLDIR)/tor/etc/config
	install -D tor/config/tor.webservices $(INSTALLDIR)/tor/etc/config
	install -D tor/config/tor.nvramconfig $(INSTALLDIR)/tor/etc/config