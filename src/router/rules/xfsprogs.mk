xfsprogs-configure:
	cd xfsprogs && ./configure --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I$(TOP)/util-linux/libuuid/src -DNEED_PRINTF" \
		LDFLAGS="$(LDLTO) -L$(TOP)/util-linux/.libs -luuid -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CC="$(CC) $(COPTS) $(MIPS16_OPT)" \
		--with-gnu-ld  \
		--disable-rpath \
		--enable-gettext=no \
		--disable-blkid \
		--enable-shared=yes \
		--enable-static=no \
		--enable-lib64=no \
		--libdir=/usr/lib \
		root_prefix=$(INSTALLDIR)/xfsprogs \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

xfsprogs:
	make -C xfsprogs DEBUG= Q= 

xfsprogs-clean:
	make -C xfsprogs clean

xfsprogs-install:
	mkdir -p $(INSTALLDIR)/xfsprogs/lib
	mkdir -p $(INSTALLDIR)/xfsprogs/usr/sbin
	cp -a $(TOP)/xfsprogs/fsck/xfs_fsck.sh $(INSTALLDIR)/xfsprogs/usr/sbin/fsck.xfs
	cp -a $(TOP)/xfsprogs/mkfs/mkfs.xfs $(INSTALLDIR)/xfsprogs/usr/sbin/mkfs.xfs
	cp -a $(TOP)/xfsprogs/repair/xfs_repair $(INSTALLDIR)/xfsprogs/usr/sbin/xfs_repair
	cp -a $(TOP)/xfsprogs/libxcmd/.libs/*.so $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libxcmd/.libs/*.so.* $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libfrog/.libs/*.so $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libfrog/.libs/*.so.* $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libhandle/.libs/*.so $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libhandle/.libs/*.so.* $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libxcmd/.libs/*.so $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libxcmd/.libs/*.so.* $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libxfs/.libs/*.so $(INSTALLDIR)/xfsprogs/lib
	cp -a $(TOP)/xfsprogs/libxfs/.libs/*.so.* $(INSTALLDIR)/xfsprogs/lib

