e2fsprogs-configure:
	cd e2fsprogs && ./configure --host=$(ARCH)-linux CFLAGS="-Os" CC="$(CROSS_COMPILE)gcc $(COPTS)" --with-gnu-ld --disable-rpath --disable-shared --enable-static --enable-elf-shlibs --enable-dynamic-e2fsck root_prefix=$(INSTALLDIR)/e2fsprogs

e2fsprogs:
	make -C e2fsprogs

e2fsprogs-install:
	make -C e2fsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/share

