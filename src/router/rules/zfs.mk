zfs-configure: libtirpc libudev openssl zlib curl ncurses util-linux
	$(MAKE) -C libtirpc
	cd zfs && ./autogen.sh
	cd zfs && autoreconf
	mkdir -p zfs/$(KERNELVERSION)
	cd zfs/$(KERNELVERSION) && ../configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--host=$(ARCH)-linux \
		--disable-pyzfs \
		CC="ccache $(CC) -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-I$(TOP)/zlib/include -I$(TOP)/util-linux/include  -I$(TOP)/util-linux/libblkid/src -I$(TOP)/util-linux/libuuid/src -I$(TOP)/curl/include -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(SSLPATH)/include  -I$(TOP)/libudev/src/libudev -D_GNU_SOURCE" \
		LDFLAGS="-L$(TOP)/zlib  -L$(TOP)/util-linux/.libs -L$(TOP)/libtirpc/src/.libs -L$(TOP)/zfs/lib/libuutil/.libs -L$(SSLPATH) -L$(TOP)/libudev/src/libudev/.libs" \
		--with-linux=$(LINUXDIR)
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec touch {} +
	touch $(TOP)/util-linux/libblkid/src/blkid.h
	touch $(SSLPATH)/include/openssl/opensslconf.h

zfs: libtirpc libudev openssl zlib ncurses util-linux
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec touch {} +
	touch $(TOP)/util-linux/libblkid/src/blkid.h
	touch $(SSLPATH)/include/openssl/opensslconf.h
	$(MAKE) -C zfs/$(KERNELVERSION) CROSS_COMPILE="ccache $(CROSS_COMPILE)"

zfs-clean:
	if test -e "zfs/$(KERNELVERSION)/Makefile"; then make -C zfs/$(KERNELVERSION) clean; fi

zfs-distclean:
	if test -e "zfs/$(KERNELVERSION)/Makefile"; then make -C zfs/$(KERNELVERSION) distclean; fi
	

zfs-install:
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec sed -i 's/relink_command/# relink_command/g' {} +
	cd zfs/$(KERNELVERSION) && find . -name "*.la" -exec touch {} +
	make -C zfs/$(KERNELVERSION) install DESTDIR=$(INSTALLDIR)/zfs
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