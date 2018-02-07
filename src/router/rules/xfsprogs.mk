xfsprogs-configure:
	cd xfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid -luuid -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS) $(MIPS16_OPT)" --with-gnu-ld --disable-rpath --enable-gettext=no --disable-blkid --enable-lib64=no root_prefix=$(INSTALLDIR)/xfsprogs

xfsprogs:
	make -C xfsprogs/libxfs crc32table.h
	make -C xfsprogs DEBUG= Q= 


xfsprogs-clean:
	make -C xfsprogs DEBUG= Q= clean

xfsprogs-install:
	-make -C xfsprogs install DESTDIR=$(INSTALLDIR)/xfsprogs PKG_SBIN_DIR=/usr/sbin PKG_ROOT_SBIN_DIR=/sbin PKG_ROOT_LIB_DIR=/lib PKG_LIB_DIR=/usr/lib	\
		PKG_MAN_DIR=/usr/man \
		PKG_LOCALE_DIR=/usr/share/locale \
		PKG_DOC_DIR=/usr/share/doc/xfsprogs
	rm -rf $(INSTALLDIR)/xfsprogs/usr/share
	rm -rf $(INSTALLDIR)/xfsprogs/usr/man
	rm -rf $(INSTALLDIR)/xfsprogs/usr/include
