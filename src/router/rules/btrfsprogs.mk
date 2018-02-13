btrfsprogs-configure: lzo
	cd btrfsprogs && ./autogen.sh
	cd btrfsprogs && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/btrfsprogs -I$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/include -I$(TOP)/e2fsprogs/lib -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/zlib -I$(TOP)/lzo/include -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib  -L$(TOP)/e2fsprogs/lib -L$(TOP)/lzo/src/.libs -lz  -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS)" --disable-backtrace --disable-documentation root_prefix=$(INSTALLDIR)/btrfsprogs  ZLIB_CFLAGS=" " ZLIB_LIBS="-lz" EXT2FS_CFLAGS=" " EXT2FS_LIBS="-lext2fs" COM_ERR_CFLAGS=" " COM_ERR_LIBS="-lcom_err" UUID_CFLAGS=" " UUID_LIBS="-luuid" BLKID_CFLAGS=" " BLKID_LIBS="-lblkid" BLKID_LIBS_STATIC="-lblkid"

btrfsprogs: lzo util-linux
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
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
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
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
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
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
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
