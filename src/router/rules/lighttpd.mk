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
        --without-valgrind 

lighttpd-configure: pcre-configure pcre openssl
	cd lighttpd && ./configure --host=$(ARCH)-linux $(CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) -I$(TOP)/pcre -I$(TOP)/zlib" CPPFLAGS="$(COPTS)" LDFLAGS="-L$(TOP)/pcre/.libs -lpthread -lpcre -L$(TOP)/zlib $(LDFLAGS)"

lighttpd: openssl
	make -C lighttpd 

lighttpd-clean:
	make -C lighttpd clean

lighttpd-install:
	install -D lighttpd/src/.libs/lighttpd $(INSTALLDIR)/lighttpd/usr/sbin/lighttpd
	mkdir -p $(INSTALLDIR)/lighttpd/usr/lib/lighttpd ; true
	cp -av lighttpd/src/.libs/mod_*.so $(INSTALLDIR)/lighttpd/usr/lib/lighttpd/ ; true
