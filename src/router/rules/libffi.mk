libffi-configure:
	cd libffi && ./configure --host=$(ARCH)-linux --libexecdir=/usr/lib --build=$(ARCH) --prefix=/usr --libdir=/usr/lib CFLAGS="$(COPTS) $(MIPS16_OPT) -std=gnu89 -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc"

libffi:
	make -C libffi

libffi-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
	-rm -rf $(INSTALLDIR)/libffi/usr/share
ifneq ($(CONFIG_PYTHON),y)
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/libffi-3.2.1
endif
	-rm -rf $(INSTALLDIR)/libffi/usr/lib/pkgconfig
	-mv $(INSTALLDIR)/libffi/usr/lib64/* $(INSTALLDIR)/libffi/usr/lib
	-rm -rf $(INSTALLDIR)/libffi/usr/lib64
	-rm -f $(INSTALLDIR)/libffi/usr/lib/*.a
	-rm -f $(INSTALLDIR)/libffi/usr/lib/*.la
