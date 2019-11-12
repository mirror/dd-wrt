transmission: libevent curl
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) -D_GNU_SOURCE -L$(TOP)/zlib -L$(TOP)/libevent/.libs -levent -L$(TOP)/curl/lib/.libs -lcurl -ldl" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto"
	$(MAKE) -C transmission

transmission-install:
	$(MAKE) -C transmission install DESTDIR=$(INSTALLDIR)/transmission
	mv $(INSTALLDIR)/transmission/usr/bin/transmission-daemon $(INSTALLDIR)/transmission/usr/bin/transmissiond
	rm -rf $(INSTALLDIR)/transmission/usr/share/man
	rm -f $(INSTALLDIR)/transmission/usr/lib/*.la
	rm -f $(INSTALLDIR)/transmission/usr/lib/*.a
	install -D transmission/configs/transmission.nvramconfig $(INSTALLDIR)/transmission/etc/config/transmission.nvramconfig
	install -D transmission/configs/transmission.webnas $(INSTALLDIR)/transmission/etc/config/06transmission.webnas
	mv -f $(INSTALLDIR)/transmission/usr/share/transmission/web $(INSTALLDIR)/transmission/usr/share/transmission/default
	cp -r transmission/combustion/ $(INSTALLDIR)/transmission/usr/share/transmission/combustion
	cp -r transmission/transmission-web-control/ $(INSTALLDIR)/transmission/usr/share/transmission/transmission-web-control

transmission-clean:
	$(MAKE) -C transmission clean

transmission-configure: libevent-configure curl-configure
	-cd transmission && ./autogen.sh
	cd transmission && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--enable-daemon --disable-nls --without-systemd-daemon --libdir=/usr/lib --disable-static --disable-dependency-tracking \
	CFLAGS="$(COPTS) -D_GNU_SOURCE -DNO_SYS_QUEUE_H -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/libevent/include -flto" \
	CPPFLAGS="$(COPTS) -D_GNU_SOURCE -DNO_SYS_QUEUE_H -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include  -I$(TOP)/openssl/include  -I$(TOP)/libevent/include -flto" \
	LDFLAGS="$(COPTS) -D_GNU_SOURCE -DNO_SYS_QUEUE_H  -L$(TOP)/zlib   -L$(TOP)/openssl -lssl -lcrypto -L$(TOP)/libevent/.libs  -L$(TOP)/curl/lib/.libs -ldl -flto" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	LIBCURL_CFLAGS="-I$(TOP)/curl/include" \
	LIBCURL_LIBS="-L$(TOP)/curl/lib/.libs -lcurl" \
	LIBEVENT_CFLAGS="-I$(TOP)/libevent/include" \
	LIBEVENT_LIBS="-L$(TOP)/libevent/.libs -levent" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto"
	$(MAKE) -C transmission clean
