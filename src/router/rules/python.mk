python-configure: libffi-configure libffi libffi-install zlib
#	cd python && cp Modules/Setup Modules/Setup
	cd python && echo "# bogus" > Modules/Setup.local 
	cd python && libtoolize -ci --force 
	cd python && aclocal
	cd python && autoreconf -fi
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
		OPT="$(COPTS) -I$(SSLPATH)/include -I$(TOP)/zlib" \
		LDFLAGS="$(COPTS) -L$(SSLPATH) -L$(TOP)/zlib -L$(TOP)/python -L$(INSTALLDIR)/libffi/usr/lib" \
		CFLAGS="$(COPTS) -I$(SSLPATH)/include -I$(TOP)/zlib -I$(INSTALLDIR)/libffi/usr/include" \
		CXXFLAGS="$(COPTS) -I$(SSLPATH)/include -I$(TOP)/zlib -I$(INSTALLDIR)/libffi/usr/include" \
		CC="ccache $(ARCH)-linux-uclibc-gcc $(COPTS)" \
		LIBFFI_INCLUDEDIR="$(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1/include" \
		ac_cv_file__dev_ptmx=yes \
		ac_cv_file__dev_ptc=no \
		ac_cv_buggy_getaddrinfo=no \
		ac_cv_header_uuid_h=yes \
		ac_cv_has_x509_verify_param_set1_host=yes



python-clean:
	make -C python clean

python: libffi zlib
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	make -C python python Parser/pgen
	make -C python sharedmods

python-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	make -C python install DESTDIR=$(INSTALLDIR)/python
	cd $(INSTALLDIR)/python/usr/bin && ln -sf /usr/bin/python3 python
	rm -rf $(INSTALLDIR)/python/usr/include
	rm -rf $(INSTALLDIR)/python/usr/share
	rm -f $(INSTALLDIR)/python/usr/lib/python3.8/config-3.8/*.a
	rm -f $(INSTALLDIR)/python/usr/lib/python3.8/config-3.8/*.o
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
	rm -rf $(INSTALLDIR)/python/usr/lib/python3.8/test

	rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
	rm -rf $(INSTALLDIR)/libffi/usr/share
	rm -f $(INSTALLDIR)/libffi/usr/lib/*.a
	rm -f $(INSTALLDIR)/libffi/usr/lib/*.la
	rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig

