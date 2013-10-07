miniupnpc:
	make -C miniupnpc OS="Linux" OS_STRING="DD-WRT" PREFIX="/usr" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC"

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

tor-configure: libevent miniupnpc-configure
	cd tor && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --with-libminiupnpc-dir=$(TOP)/libminiupnpc --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--enable-upnp \
	--disable-asciidoc \
	--disable-gcc-hardening \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(TOP)/miniupnpc  -I$(TOP)/openssl/include -I$(TOP)/libevent -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib -I$(TOP) -I$(TOP)/miniupnpc -I$(TOP)/openssl/include -I$(TOP)/libevent  -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib   -L$(TOP)/openssl -L$(TOP)/libevent/.libs -L$(TOP)/miniupnpc" 

tor: libevent miniupnpc
	make -C tor

tor-clean:
	make -C tor clean

tor-install:
	make -C tor DESTDIR=$(INSTALLDIR)/tor install
	install -D tor/config/torrc $(INSTALLDIR)/tor/usr/etc/tor