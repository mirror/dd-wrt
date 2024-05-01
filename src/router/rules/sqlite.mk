sqlite-configure:
	cd sqlite && ./configure --host=$(ARCH)-linux --disable-readline --prefix=/usr --libdir=/usr/lib --disable-static-shell CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC"

sqlite:
	make -C sqlite

sqlite-install:
	$(MAKE) -C sqlite install DESTDIR=$(INSTALLDIR)/sqlite
	rm -rf $(INSTALLDIR)/sqlite/usr/include
	rm -rf $(INSTALLDIR)/sqlite/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/sqlite/usr/lib/*.la
	rm -rf $(INSTALLDIR)/sqlite/usr/lib/*.a
	rm -rf $(INSTALLDIR)/sqlite/usr/share
	rm -rf $(INSTALLDIR)/sqlite/usr/bin
