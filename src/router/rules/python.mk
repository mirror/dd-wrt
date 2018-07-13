python-configure: libffi-configure libffi libffi-install
	cd python && ./configure  --host=$(ARCH)-linux --build=$(ARCH) --sysconfdir=/etc \
		--enable-shared \
		--enable-static \
		--prefix=/usr \
		--enable-optimizations \
		--disable-profiling \
		--without-cxx-main \
		--with-system-ffi="$(INSTALLDIR)/libffi/usr" \
		--with-threads \
		--without-ensurepip \
		--enable-ipv6 \
		CONFIG_SITE="$(TOP)/python/site/config.site" \
		OPT="$(COPTS) -I$(TOP)/openssl/include -I$(TOP)/zlib" \
		LDFLAGS="$(COPTS) -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/python" \
		CFLAGS="$(COPTS) -I$(TOP)/openssl/include -I$(TOP)/zlib -I$(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1/include" \
		CXXFLAGS="$(COPTS) -I$(TOP)/openssl/include -I$(TOP)/zlib -I$(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1/include" \
		CC="ccache $(ARCH)-linux-uclibc-gcc $(COPTS)" \
		LIBFFI_INCLUDEDIR="$(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1/include" \
		ac_cv_file__dev_ptmx=yes \
		ac_cv_file__dev_ptc=no \
		ac_cv_buggy_getaddrinfo=no \
		ac_cv_header_uuid_h=yes \
		ac_cv_has_x509_verify_param_set1_host=yes



python-clean:
	make -C python clean

python: libffi
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	make -C python python Parser/pgen
	make -C python sharedmods

python-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	make -C python install DESTDIR=$(INSTALLDIR)/python
	rm -rf $(INSTALLDIR)/python/usr/include
	rm -rf $(INSTALLDIR)/python/usr/share
	rm -f $(INSTALLDIR)/python/usr/lib/python3.6/config-3.6m/*.a
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
	rm -rf $(INSTALLDIR)/python/usr/lib/python3.6/test

	rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
	rm -rf $(INSTALLDIR)/libffi/usr/share
	rm -f $(INSTALLDIR)/libffi/usr/lib/*.a
	rm -f $(INSTALLDIR)/libffi/usr/lib/*.la
	rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig

