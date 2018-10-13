libtirpc-configure:
	cd libtirpc && ./bootstrap
	cd libtirpc && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib" --disable-gssapi --disable-static --prefix=/usr

libtirpc:
	make -C libtirpc

libtirpc-clean:
	make -C libtirpc clean


zfs-configure:
	cd zfsnew && ./configure \
		--prefix=/usr \
		--host=$(ARCH)-linux \
		CC="$(CC) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-I$(TOP)/zlib -I$(TOP)/e2fsprogs/lib  -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(TOP)/openssl/include" \
		LDFLAGS="-L$(TOP)/zlib  -L$(TOP)/e2fsprogs/lib/blkid -L$(TOP)/e2fsprogs/lib/uuid -L$(TOP)/libtirpc/src/.libs -L$(TOP)/openssl" \
		--with-linux=$(LINUXDIR)

zfs:
	$(MAKE) -j 4 -C zfsnew

zfs-clean:
	if test -e "zfsnew/Makefile"; then make -C zfsnew clean; fi
	

zfs-install:
	make -C libtirpc install DESTDIR=$(INSTALLDIR)
	make -C zfs install DESTDIR=$(INSTALLDIR)
