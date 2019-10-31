gnutls-configure: gmp nettle
	cd nettle && make install DESTDIR=$(TOP)/_staging
	cd gnutls && autoreconf --force --install --symlink
	cd gnutls && ./configure --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF"  CXXFLAGS="$(COPTS) -fPIC -DNEED_PRINTF"  CPPFLAGS="$(COPTS) -fPIC -DNEED_PRINTF" \
		--with-included-unistring \
		--with-included-libtasn1 \
		--libdir=/usr/lib \
		--without-p11-kit \
		NETTLE_CFLAGS="-I$(TOP)/_staging/usr/include" \
		NETTLE_LIBS="-L$(TOP)/nettle -lnettle -L$(TOP)/gmp/.libs -lgmp" \
		HOGWEED_CFLAGS="-I$(TOP)/_staging/usr/include" \
		HOGWEED_LIBS="-L$(TOP)/nettle -lhogweed" \
		GMP_CFLAGS="-I$(TOP)/gmp" \
		GMP_LIBS="-L$(TOP)/gmp/.libs -lgmp"

gnutls: gmp nettle
	cd nettle && make install DESTDIR=$(TOP)/_staging
	make -C gnutls

gnutls-install:
	$(MAKE) -C gmp install DESTDIR=$(INSTALLDIR)/gnutls
	$(MAKE) -C nettle install DESTDIR=$(INSTALLDIR)/gnutls
	$(MAKE) -C gnutls install DESTDIR=$(INSTALLDIR)/gnutls
	mkdir -p $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutls.so $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutls.so.30 $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutls.so.30.26.0 $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutlsxx.so $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutlsxx.so.28 $(INSTALLDIR)/gnutls/usr/lib
	cp -a gnutls/lib/.libs/libgnutlsxx.so.28.1.0 $(INSTALLDIR)/gnutls/usr/lib


	rm -rf $(INSTALLDIR)/gnutls/usr/include
	rm -rf $(INSTALLDIR)/gnutls/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/gnutls/usr/lib/*.la
	rm -rf $(INSTALLDIR)/gnutls/usr/lib/*.a
	rm -rf $(INSTALLDIR)/gnutls/usr/share
