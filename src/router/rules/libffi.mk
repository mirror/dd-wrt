libffi-configure:
	cd libffi && ./configure --host=$(ARCH)-linux --build=$(ARCH) --prefix=/usr --libdir=/usr/lib CFLAGS="$(COPTS)"

libffi:
	make -C libffi

libffi-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
