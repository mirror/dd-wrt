e2fsprogs-configure:
	touch e2fsprogs/intl/plural.c
	cd e2fsprogs && ./configure \
		    --host=$(ARCH)-linux \
		    CC="$(CC) $(COPTS) $(MIPS16_OPT) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE  -DNEED_PRINTF -std=gnu89 -I$(TOP)/e2fsprogs/intl -I$(TOP)/util-linux/libblkid/src -I$(TOP)/util-linux/libuuid/src" \
		    LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/e2fsprogs/intl -L$(TOP)/util-linux/.libs" \
		    --disable-static \
		    --enable-shared \
		    --with-gnu-ld \
		    --disable-rpath \
		    --enable-elf-shlibs \
		    --enable-compression \
		    --enable-htree \
		    --enable-symlink-install \
		    --disable-tls \
		    --libdir=/lib \
		    root_prefix=$(INSTALLDIR)/e2fsprogs \
		    --disable-nls \
		    --disable-libuuid \
		    --enable-libuuid=no \
		    --disable-uuidd \
		    --disable-debugfs
	
	make -C e2fsprogs clean
	make -C e2fsprogs

e2fsprogs:
	make -C e2fsprogs

e2fsprogs-clean:
	make -C e2fsprogs clean

e2fsprogs-install:
	make -C e2fsprogs install DESTDIR=$(INSTALLDIR)/e2fsprogs LDCONFIG=true

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
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/fsck*
	rm -f $(INSTALLDIR)/e2fsprogs/lib/e2initrd_helper	
endif
	rm -f $(INSTALLDIR)/e2fsprogs/lib/libss*
	rm -rf $(INSTALLDIR)/e2fsprogs/etc/cron.d
	rm -f $(INSTALLDIR)/e2fsprogs/lib/*.a
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/debugfs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/blkid
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/dumpe2fs
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2image
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2undo
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/e2mmp*
	rm -f $(INSTALLDIR)/e2fsprogs/sbin/logsave
	rm -f $(INSTALLDIR)/e2fsprogs/usr/sbin/uuidd
	rm -f $(INSTALLDIR)/e2fsprogs/usr/sbin/mklost+found
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/share
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/bin
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/include
	rm -rf $(INSTALLDIR)/e2fsprogs/lib/pkgconfig
	rm -rf $(INSTALLDIR)/e2fsprogs/usr/man
	-cd $(INSTALLDIR)/e2fsprogs/etc && ln -sf /proc/mounts mtab
	true
