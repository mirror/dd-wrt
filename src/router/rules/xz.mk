xz-configure:
	-cd xz && ./autogen.sh
	cd xz && sh ./configure --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -std=c99" ac_cv_prog_cc_c99= \
	--libdir=/usr/lib \
	--enable-small \
	--enable-assume-ram=4 \
	--disable-werror \
	--disable-doc

xz:
	$(MAKE) -C xz

xz-clean:
	$(MAKE) -C xz clean

xz-install:
	$(MAKE) -C xz install DESTDIR=$(INSTALLDIR)/xz
	rm -rf $(INSTALLDIR)/xz/usr/include
	rm -rf $(INSTALLDIR)/xz/usr/bin
	rm -rf $(INSTALLDIR)/xz/usr/share
	rm -rf $(INSTALLDIR)/xz/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/xz/usr/lib/*.a
	rm -f $(INSTALLDIR)/xz/usr/lib/*.la