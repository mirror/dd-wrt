e2fsprogs-configure:
	cd e2fsprogs && ./configure --host=$(ARCH)-linux CC="$(CC) $(COPTS)  -DNEED_PRINTF" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-elf-shlibs --enable-compression --enable-htree --enable-symlink-install --disable-tls --libdir=/lib root_prefix=$(INSTALLDIR)/e2fsprogs
	make -C e2fsprogs
	cd xfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid" CC="$(CC) $(COPTS)" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-gettext=no --enable-lib64=no root_prefix=$(INSTALLDIR)/e2fsprogs

e2fsprogs:
	make -C e2fsprogs
ifeq ($(CONFIG_E2FSPROGS_ADV),y)
	make -C xfsprogs DEBUG= Q= 
	make -C btrfsprogs CC="$(CC)" CFLAGS="$(COPTS) -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid  -ffunction-sections -fdata-sections -Wl,--gc-sections" prefix=/usr
endif

e2fsprogs-clean:
	make -C e2fsprogs clean
ifeq ($(CONFIG_E2FSPROGS_ADV),y)
	make -C xfsprogs DEBUG= Q= clean
	make -C btrfsprogs CC="$(CC)" CFLAGS="$(COPTS) -I$(TOP)/e2fsprogs/lib -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid  -ffunction-sections -fdata-sections -Wl,--gc-sections" prefix=/usr clean
endif

e2fsprogs-install:
	-make -C e2fsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs
ifeq ($(CONFIG_E2FSPROGS_ADV),y)
	-make -C xfsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs PKG_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/usr/sbin PKG_ROOT_SBIN_DIR=$(INSTALLDIR)/e2fsprogs/sbin PKG_ROOT_LIB_DIR=$(INSTALLDIR)/e2fsprogs/lib PKG_LIB_DIR=$(INSTALLDIR)/e2fsprogs/usr/lib	\
		PKG_MAN_DIR=$(INSTALLDIR)/e2fsprogs/usr/man \
		PKG_LOCALE_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/locale \
		PKG_DOC_DIR=$(INSTALLDIR)/e2fsprogs/usr/share/doc/xfsprogs
	-make -C btrfsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs prefix=/usr
else
	-rm -f $(INSTALLDIR)/e2fsprogs/sbin/*fsck*
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/bin
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/sbin
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/lib
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/debugfs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/dumpe2fs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2image
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/badblocks
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/blkid
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/logsave
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/resize2fs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/tune2fs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2undo
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2label
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/findfs
endif
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/share
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/man
	-cd $(INSTALLDIR)/e2fsprogs/etc && ln -sf /proc/mounts mtab
	true
