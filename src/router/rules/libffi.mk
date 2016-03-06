libffi-configure:
	cd libffi && ./configure --host=$(ARCH)-linux --build=$(ARCH) --prefix=/usr --libdir=/usr/lib CFLAGS="$(COPTS) -std=gnu89 -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc"

libffi:
	make -C libffi

libffi-install:
	make -C libffi install DESTDIR=$(INSTALLDIR)/libffi
