libmnl: 
	$(MAKE) -C libmnl

libmnl-install:
	@true
	install -D libmnl/src/.libs/libmnl.so.0.2.0 $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.2.0
	cd $(INSTALLDIR)/libmnl/usr/lib ; ln -s libmnl.so.0.2.0 libmnl.so  ; true
	cd $(INSTALLDIR)/libmnl/usr/lib ; ln -s libmnl.so.0.2.0 libmnl.so.0  ; true

libmnl-configure:
	-cd libmnl && aclocal && autoconf && automake -a && cd ..
	cd libmnl && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)"

libmnl-clean:
	$(MAKE) -C libmnl clean

