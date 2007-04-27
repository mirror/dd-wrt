e2fsprogs:
	cd e2fsprogs && ./configure --host=$(ARCH)-linux CFLAGS="-Os" CC="$(CROSS_COMPILE)gcc $(COPTS)" --with-gnu-ld --disable-rpath --disable-shared --enable-static --enable-elf-shlibs --enable-dynamic-e2fsck
	make -C e2fsprogs

e2fsprogs-install:
	mkdir -p $(INSTALLDIR)/e2fsprogs/sbin
	cp e2fsprogs/misc/mke2fs.static $(INSTALLDIR)/e2fsprogs/sbin/mke2fs

