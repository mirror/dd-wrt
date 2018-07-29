transmission: libevent curl
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -L$(TOP)/zlib -L$(TOP)/libevent/.libs -levent -L$(TOP)/curl/lib/.libs -lcurl -ldl" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto"
	$(MAKE) -C transmission

transmission-install:
	$(MAKE) -C transmission install DESTDIR=$(INSTALLDIR)/transmission
	mv $(INSTALLDIR)/transmission/usr/bin/transmission-daemon $(INSTALLDIR)/transmission/usr/bin/transmissiond
	rm -rf $(INSTALLDIR)/transmission/usr/share/man
	install -D transmission/configs/transmission.nvramconfig $(INSTALLDIR)/transmission/etc/config/transmission.nvramconfig
	install -D transmission/configs/transmission.webnas $(INSTALLDIR)/transmission/etc/config/transmission.webnas

transmission-clean:
	$(MAKE) -C transmission clean

transmission-configure: libevent-configure curl-configure
	-cd transmission && ./autogen.sh
	cd transmission && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--enable-daemon --disable-nls --without-systemd-daemon \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -DNO_SYS_QUEUE_H -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -DNO_SYS_QUEUE_H -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include  -I$(TOP)/openssl/include  -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -DNO_SYS_QUEUE_H  -L$(TOP)/zlib   -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/libevent/.libs  -L$(TOP)/curl/lib/.libs -ldl" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	LIBCURL_CFLAGS="-I$(TOP)/curl/include" \
	LIBCURL_LIBS="-L$(TOP)/curl/lib/.libs -lcurl" \
	LIBEVENT_CFLAGS="-I$(TOP)/libevent/include" \
	LIBEVENT_LIBS="-L$(TOP)/libevent/.libs -levent" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto"
	$(MAKE) -C transmission clean
