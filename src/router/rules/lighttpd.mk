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
        --without-libev \
        --without-memcache \
        --without-mysql \
	--with-libpcre-includes="$(TOP)/pcre" \
	--with-libpcre_libraries="$(TOP)/pcre/.libs" \
        --with-openssl="$(SSLPATH)" \
	--with-openssl-includes="$(SSLPATH)/include" \
	--with-openssl-libs="$(SSLPATH)" \
        --with-pcre \
        --without-valgrind \
        --with-zlib

lighttpd-configure: pcre-configure pcre openssl zlib
	cd lighttpd && autoreconf -fiv || exit 1
	cd lighttpd && rm -Rf autom4te.cache
	cd lighttpd && ./configure --host=$(ARCH)-linux $(CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/pcre -I$(TOP)/zlib" CPPFLAGS="$(COPTS) $(MIPS16_OPT)" LDFLAGS="-L$(TOP)/pcre/.libs -lpthread -lpcre -L$(TOP)/zlib $(LDFLAGS) -lz" PCRE_LIB="-lpcre" PCRECONFIG="true"

lighttpd: openssl zlib
	install -D lighttpd/configs/lighttpd.webservices httpd/ej_temp/lighttpd.webservices
	make -C lighttpd 

lighttpd-clean:
	-make -C lighttpd clean

lighttpd-install:
	install -D lighttpd/src/.libs/lighttpd $(INSTALLDIR)/lighttpd/usr/sbin/lighttpd
	install -D $(TOP)/pcre/.libs/libpcre.so.1 $(INSTALLDIR)/lighttpd/usr/lib/libpcre.so.1
	mkdir -p $(INSTALLDIR)/lighttpd/usr/lib/lighttpd ; true
	cp -av lighttpd/src/.libs/mod_*.so $(INSTALLDIR)/lighttpd/usr/lib/lighttpd/ ; true
	mkdir -p $(INSTALLDIR)/lighttpd/etc ; true
	install -D lighttpd/configs/lighttpd.nvramconfig $(INSTALLDIR)/lighttpd/etc/config/lighttpd.nvramconfig
	install -D lighttpd/configs/lighttpd.webservices $(INSTALLDIR)/lighttpd/etc/config/lighttpd.webservices
	cat $(TOP)/httpd/cert.pem $(TOP)/httpd/key.pem > $(INSTALLDIR)/lighttpd/etc/host.pem ; true
