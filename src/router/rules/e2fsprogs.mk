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

btrfsprogs-configure:
	cd btrfsprogs && ./autogen.sh
	cd btrfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/btrfsprogs -I$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/include -I$(TOP)/e2fsprogs/lib -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/zlib -I$(TOP)/lzo/include -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib  -L$(TOP)/e2fsprogs/lib -L$(TOP)/lzo/src/.libs -lz  -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS)" --disable-backtrace --disable-documentation root_prefix=$(INSTALLDIR)/btrfsprogs  ZLIB_CFLAGS=" " ZLIB_LIBS="$(TOP)/zlib/libz.a" EXT2FS_CFLAGS=" " EXT2FS_LIBS="-lext2fs" COM_ERR_CFLAGS=" " COM_ERR_LIBS="-lcom_err" BLKID_CFLAGS=" " BLKID_LIBS="-lblkid"

btrfsprogs: util-linux
	make -C util-linux clean
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
	rm -f $(TOP)/util-linux/.libs/libuuid.a
	rm -f $(TOP)/util-linux/.libs/libblkid.so*
	make -C btrfsprogs 

btrfsprogs-clean:
	make -C btrfsprogs clean

btrfsprogs-install:	
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
	rm -f $(TOP)/util-linux/.libs/libuuid.a
	rm -f $(TOP)/util-linux/.libs/libblkid.so*
	-make -C btrfsprogs install DESTDIR=$(INSTALLDIR)/btrfsprogs prefix=/usr
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/include
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/lib
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/share
	
e2fsprogs-configure:
	cd e2fsprogs && ./configure --host=$(ARCH)-linux CC="$(CC) $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE  -DNEED_PRINTF -std=gnu89" --disable-static --enable-shared --with-gnu-ld --disable-rpath --enable-elf-shlibs --enable-compression --enable-htree --enable-symlink-install --disable-tls --libdir=/lib root_prefix=$(INSTALLDIR)/e2fsprogs
	make -C e2fsprogs

e2fsprogs:
	make -C e2fsprogs

e2fsprogs-clean:
	make -C e2fsprogs clean

e2fsprogs-install:
	-make -C e2fsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs

ifneq ($(CONFIG_E2FSPROGS_ADV),y)
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
