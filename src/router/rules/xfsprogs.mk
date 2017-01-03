xfsprogs-configure:
	cd xfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid  -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS)" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-gettext=no --enable-lib64=no root_prefix=$(INSTALLDIR)/xfsprogs

xfsprogs:
	make -C xfsprogs DEBUG= Q= 


xfsprogs-clean:
	make -C xfsprogs DEBUG= Q= clean

xfsprogs-install:
	-make -C xfsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs PKG_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/usr/sbin PKG_ROOT_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/sbin PKG_ROOT_LIB_DIR=$(INSTALLDIR)/e2fsprogs/lib PKG_LIB_DIR=$(INSTALLDIR)/e2fsprogs/usr/lib	\
		PKG_MAN_DIR=$(INSTALLDIR)/e2fsprogs/usr/man \
		PKG_LOCALE_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/locale \
		PKG_DOC_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/doc/xfsprogs
