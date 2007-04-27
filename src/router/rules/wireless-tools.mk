wireless-tools-clean:
	make -C wireless-tools clean

wireless-tools:
	make -C wireless-tools CC=$(CC) CFLAGS="$(COPTS)" all iwmulticall

wireless-tools-install:
	make -C wireless-tools INSTALL_DIR=$(INSTALLDIR)/wireless-tools/usr/sbin install-bin
	make -C wireless-tools INSTALL_LIB=$(INSTALLDIR)/wireless-tools/lib install-dynamic
	

