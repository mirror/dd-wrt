libffi-configure:
	cd libffi && ./configure --host=$(ARCH)-linux --build=$(ARCH) --prefix=/usr --libdir=/usr/lib CFLAGS="$(COPTS) -std=gnu89 -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc"

libffi:
	make -C libffi

libffi-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	-rm -rf $(INSTALLDIR)/libffi/usr/share
ifneq ($(CONFIG_PYTHON),y)
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
endif
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig
	-rm -f $(INSTALLDIR)/libffi/usr/lib/*.a
	-rm -f $(INSTALLDIR)/libffi/usr/lib/*.la
