uhttpd-configure:
	@true

uhttpd: cyassl
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C uhttpd compile
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/aoss2/tools clean
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/aoss2/tools

uhttpd-clean:
	make -C uhttpd clean
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/aoss2/tools clean

uhttpd-install:
	install -D uhttpd/uhttpd $(INSTALLDIR)/uhttpd/usr/sbin/uhttpd
	install -D uhttpd/uhttpd_tls.so $(INSTALLDIR)/uhttpd/usr/lib/uhttpd_tls.so
	install -D private/buffalo/aoss2/tools/cryptvalue $(INSTALLDIR)/uhttpd/usr/sbin/cryptvalue
	install -D private/buffalo/aoss2/tools/upnp_discover $(INSTALLDIR)/uhttpd/usr/sbin/upnp_discover

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
        --with-openssl="$(SSLPATH)" \
	--with-openssl-includes="$(SSLPATH)/include" \
	--with-openssl-libs="$(SSLPATH)" \
        --with-pcre \
        --without-valgrind 

lighttpd-configure: pcre-configure pcre openssl
	cd lighttpd && ./configure $(CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) -I$(TOP)/pcre" LDFLAGS="-L$(TOP)/pcre/.libs -lpthread $(LDFLAGS)"

lighttpd: openssl
	make -C lighttpd

lighttpd-clean:
	make -C lighttpd clean

lighttpd-install:
	install -D lighttpd/src/.libs/lighttpd $(INSTALLDIR)/lighttpd/usr/sbin/lighttpd
	mkdir -p $(INSTALLDIR)/lighttpd/usr/lib/lighttpd ; true
	cp -av lighttpd/src/.libs/mod_*.so $(INSTALLDIR)/lighttpd/usr/lib/lighttpd/ ; true
