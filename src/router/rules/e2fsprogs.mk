e2fsprogs-configure:
	cd e2fsprogs && ./configure --host=$(ARCH)-linux CFLAGS="-Os" CC="$(CROSS_COMPILE)gcc $(COPTS)  -DNEED_PRINTF" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-elf-shlibs --enable-compression --enable-htree root_prefix=$(INSTALLDIR)/e2fsprogs
	make -C e2fsprogs
	cd xfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="-Os -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid" CC="$(CROSS_COMPILE)gcc $(COPTS)" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-gettext=no --enable-lib64=no root_prefix=$(INSTALLDIR)/e2fsprogs

e2fsprogs:
	make -C e2fsprogs
	make -C xfsprogs DEBUG= Q=
	make -C btrfsprogs CC="$(CROSS_COMPILE)gcc" CFLAGS="$(COPTS) -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid  -ffunction-sections -fdata-sections -Wl,--gc-sections" prefix=/usr

e2fsprogs-install:
	-make -C e2fsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs
	-make -C xfsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs PKG_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/usr/sbin PKG_ROOT_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/sbin PKG_ROOT_LIB_DIR=$(INSTALLDIR)/e2fsprogs/lib PKG_LIB_DIR=$(INSTALLDIR)/e2fsprogs/usr/lib	\
		PKG_MAN_DIR=$(INSTALLDIR)/e2fsprogs/usr/man \
		PKG_LOCALE_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/locale \
		PKG_DOC_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/doc/xfsprogs
	-make -C btrfsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs prefix=/usr
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/share
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/man
