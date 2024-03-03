libressl-configure:
	cd libressl && libtoolize -ci --force 
	cd libressl && aclocal
	cd libressl && automake --add-missing
	cd libressl && autoreconf -fi 
	cd libressl && ./configure  --prefix=/usr --libdir=/usr/lib ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) --disable-static --enable-shared --disable-hardening --disable-tests

libressl:
	make -C libressl
	make -C libressl DESTDIR=$(TOP)/libressl/tmpinstall install
	mkdir -p $(TOP)/libressl/tmpinstall/usr/sbin
	rm -f $(TOP)/libressl/tmpinstall/usr/bin/ocspcheck
	rm -rf $(TOP)/libressl/tmpinstall/usr/etc
	rm -rf $(TOP)/libressl/tmpinstall/usr/include
	rm -rf $(TOP)/libressl/tmpinstall/usr/share
	rm -f $(TOP)/libressl/tmpinstall/usr/lib/*.la
	rm -f $(TOP)/libressl/tmpinstall/usr/lib/*.a
	rm -rf $(TOP)/libressl/tmpinstall/usr/lib/pkgconfig
	mv -f $(TOP)/libressl/tmpinstall/usr/bin/openssl $(TOP)/libressl/tmpinstall/usr/sbin
	cp -a $(TOP)/libressl/tmpinstall/usr/lib/* $(TOP)/libressl/
	

libressl-clean:
	make -C libressl clean

libressl-install:
	make -C libressl DESTDIR=$(INSTALLDIR)/libressl install
	mkdir -p $(INSTALLDIR)/libressl/usr/sbin
	rm -f $(INSTALLDIR)/libressl/usr/bin/ocspcheck
	rm -rf $(INSTALLDIR)/libressl/usr/etc
	rm -rf $(INSTALLDIR)/libressl/usr/include
	rm -rf $(INSTALLDIR)/libressl/usr/share
	rm -f $(INSTALLDIR)/libressl/usr/lib/*.la
	rm -f $(INSTALLDIR)/libressl/usr/lib/*.a
	rm -rf $(INSTALLDIR)/libressl/usr/lib/pkgconfig
	mv -f $(INSTALLDIR)/libressl/usr/bin/openssl $(INSTALLDIR)/libressl/usr/sbin
