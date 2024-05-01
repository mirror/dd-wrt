btrfsprogs-configure: lzo zstd zlib
	cd btrfsprogs && ./autogen.sh
	cd btrfsprogs && ./configure --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/btrfsprogs -I$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/include -I$(TOP)/e2fsprogs/lib -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/zlib -I$(TOP)/lzo/include -DNEED_PRINTF -I$(TOP)/zstd/lib" \
		LDFLAGS="$(LDLTO) -L$(TOP)/zlib -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib  -L$(TOP)/e2fsprogs/lib -L$(TOP)/lzo/src/.libs -lz -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CC="$(CC) $(COPTS)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
		--disable-static \
		--enable-shared \
		--disable-python \
		--disable-backtrace \
		--disable-documentation \
		root_prefix=$(INSTALLDIR)/btrfsprogs \
		ZLIB_CFLAGS=" " \
		ZLIB_LIBS="-lz" \
		EXT2FS_CFLAGS=" " \
		EXT2FS_LIBS="-lext2fs" \
		COM_ERR_CFLAGS=" " \
		COM_ERR_LIBS="-lcom_err" \
		UUID_CFLAGS=" " \
		UUID_LIBS="-luuid" \
		BLKID_CFLAGS=" " \
		BLKID_LIBS="-lblkid" \
		BLKID_LIBS_STATIC="-lblkid" \
		ZSTD_CFLAGS="-I$(TOP)/zstd/lib" \
		ZSTD_LIBS="-L$(TOP)/zstd/lib -lzstd"


btrfsprogs: zstd lzo util-linux zlib
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
	make -C btrfsprogs all
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_ZFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*



btrfsprogs-clean:
	make -C btrfsprogs clean

btrfsprogs-install:	
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	-make -C btrfsprogs install DESTDIR=$(INSTALLDIR)/btrfsprogs prefix=/usr
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	#-rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfs*
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/include
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/lib
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/lib64
	-rm -rf $(INSTALLDIR)/btrfsprogs/usr/share
	rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfs-*
	rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfck
	#rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfs
	#rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/fsck.btrfs
	#rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfsck
	#rm -f $(INSTALLDIR)/btrfsprogs/usr/bin/btrfstune
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/etc
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*
