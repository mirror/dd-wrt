libtirpc-configure:
	cd libtirpc && ./bootstrap
	cd libtirpc && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib" --disable-gssapi --disable-static --prefix=/usr

libtirpc:
	make -C libtirpc

libtirpc-clean:
	make -C libtirpc clean

libtirpc-install:
	make -C libtirpc install DESTDIR=$(INSTALLDIR)/libtirpc
	rm -rf $(INSTALLDIR)/libtirpc/usr/include
	rm -rf $(INSTALLDIR)/libtirpc/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libtirpc/usr/share
	rm -f $(INSTALLDIR)/libtirpc/usr/lib/*.a
	rm -f $(INSTALLDIR)/libtirpc/usr/lib/*.la


zfs-configure: libtirpc-configure libtirpc libudev
	cd zfs && ./autogen.sh
	cd zfs && autoreconf
	cd zfs && ./configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--host=$(ARCH)-linux \
		CC="$(CC) -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-I$(TOP)/zlib -I$(TOP)/e2fsprogs/lib  -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(TOP)/openssl/include  -I$(TOP)/libudev/src/libudev" \
		LDFLAGS="-L$(TOP)/zlib  -L$(TOP)/e2fsprogs/lib/blkid -L$(TOP)/e2fsprogs/lib/uuid -L$(TOP)/libtirpc/src/.libs -L$(TOP)/zfs/lib/libuutil/.libs -L$(TOP)/openssl -L$(TOP)/libudev/src/libudev/.libs -ludev" \
		--with-linux=$(LINUXDIR)
	cd zfs && find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfs && find . -name *.la -exec touch {} +

zfs: libtirpc libudev
	$(MAKE) -j 4 -C zfs

zfs-clean:
	if test -e "zfs/Makefile"; then make -C zfs clean; fi

zfs-distclean:
	if test -e "zfs/Makefile"; then make -C zfs distclean; fi
	

zfs-install:
	cd zfs && find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfs && find . -name *.la -exec touch {} +
	make -C zfs install DESTDIR=$(INSTALLDIR)/zfs
	rm -rf $(INSTALLDIR)/zfs/usr/include
	rm -rf $(INSTALLDIR)/zfs/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/zfs/usr/share
	rm -rf $(INSTALLDIR)/zfs/usr/etc
	rm -rf $(INSTALLDIR)/zfs/usr/lib/dracut
	rm -rf $(INSTALLDIR)/zfs/usr/lib/systemd
	rm -rf $(INSTALLDIR)/zfs/usr/lib/modules-load.d
	rm -rf $(INSTALLDIR)/zfs/usr/libexec
	rm -rf $(INSTALLDIR)/zfs/usr/src
	rm -f $(INSTALLDIR)/zfs/usr/lib/*.a
	rm -f $(INSTALLDIR)/zfs/usr/lib/*.la
	mkdir -p $(INSTALLDIR)/zfs/lib2/modules/$(KERNELRELEASE)
	find $(INSTALLDIR)/zfs/lib/modules  -name *.ko -exec mv {} $(INSTALLDIR)/zfs/lib2/modules/$(KERNELRELEASE) \;
	rm -rf $(INSTALLDIR)/zfs/lib
	mv $(INSTALLDIR)/zfs/lib2 $(INSTALLDIR)/zfs/lib