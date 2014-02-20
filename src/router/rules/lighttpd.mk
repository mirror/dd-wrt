CONFIGURE_ARGS+= \
        --libdir=/usr/lib/lighttpd \
        --sysconfdir=/tmp/lighttpd \
        --enable-shared \
        --enable-static \
        --disable-rpath \
        --without-attr \
        --without-bzip2 \
        --without-fam \
        --without-gdbm \
        --without-ldap \
        --without-lua \
        --without-memcache \
        --without-mysql \
	--with-libpcre-includes="$(TOP)/pcre" \
	--with-libpcre_libraries="$(TOP)/pcre/.libs" \
        --with-openssl="$(TOP)/openssl" \
	--with-openssl-includes="$(TOP)/openssl/include" \
	--with-openssl-libs="$(TOP)/openssl" \
        --with-pcre \
        --without-valgrind \
        --with-zlib

lighttpd-configure: pcre-configure pcre openssl
	cd lighttpd && ./configure --host=$(ARCH)-linux CC="ccache $(CC)" $(CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/pcre -I$(TOP)/zlib" CPPFLAGS="$(COPTS) $(MIPS16_OPT)" LDFLAGS="-L$(TOP)/pcre/.libs -lpthread -lpcre -L$(TOP)/zlib $(LDFLAGS) -lz"

lighttpd: openssl
	make -C lighttpd 

lighttpd-clean:
	make -C lighttpd clean

lighttpd-install:
	install -D lighttpd/src/.libs/lighttpd $(INSTALLDIR)/lighttpd/usr/sbin/lighttpd
	install -D $(TOP)/pcre/.libs/libpcre.so.1 $(INSTALLDIR)/lighttpd/usr/lib/libpcre.so.1
	mkdir -p $(INSTALLDIR)/lighttpd/usr/lib/lighttpd ; true
	cp -av lighttpd/src/.libs/mod_*.so $(INSTALLDIR)/lighttpd/usr/lib/lighttpd/ ; true
	mkdir -p $(INSTALLDIR)/lighttpd/etc ; true
	install -D lighttpd/configs/lighttpd.nvramconfig $(INSTALLDIR)/lighttpd/etc/config/lighttpd.nvramconfig
	install -D lighttpd/configs/lighttpd.webserver $(INSTALLDIR)/lighttpd/etc/config/lighttpd.webserver
	cat $(TOP)/httpd/cert.pem $(TOP)/httpd/key.pem > $(INSTALLDIR)/lighttpd/etc/host.pem ; true
