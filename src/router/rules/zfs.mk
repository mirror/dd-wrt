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


zfs-configure: libtirpc-configure libtirpc
	cd zfsnew && ./autogen.sh
	cd zfsnew && autoreconf
	cd zfsnew && ./configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--host=$(ARCH)-linux \
		CC="$(CC) -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-I$(TOP)/zlib -I$(TOP)/e2fsprogs/lib  -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(TOP)/openssl/include" \
		LDFLAGS="-L$(TOP)/zlib  -L$(TOP)/e2fsprogs/lib/blkid -L$(TOP)/e2fsprogs/lib/uuid -L$(TOP)/libtirpc/src/.libs -L$(TOP)/zfsnew/lib/libuutil/.libs -L$(TOP)/openssl" \
		--with-linux=$(LINUXDIR)
	cd zfsnew && find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfsnew && find . -name *.la -exec touch {} +

zfs: libtirpc
	$(MAKE) -j 4 -C zfsnew

zfs-clean:
	if test -e "zfsnew/Makefile"; then make -C zfsnew clean; fi

zfs-distclean:
	if test -e "zfsnew/Makefile"; then make -C zfsnew distclean; fi
	

zfs-install:
	cd zfsnew && find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfsnew && find . -name *.la -exec touch {} +
	make -C zfsnew install DESTDIR=$(INSTALLDIR)/zfs
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