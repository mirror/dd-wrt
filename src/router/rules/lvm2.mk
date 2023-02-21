lvm2-configure: e2fsprogs
	cd lvm2 && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" \
		BLKID_CFLAGS="-L$(TOP)/util-linux/libblkid/src" \
		BLKID_LIBS="-L$(TOP)/util-linux/.libs -lblkid"

lvm2: e2fsprogs
	make -C lvm2 device-mapper

lvm2-clean:
	make -C lvm2 clean

lvm2-install:
	make -C lvm2 install_device-mapper DESTDIR=$(INSTALLDIR)/lvm2
	rm -rf $(INSTALLDIR)/lvm2/usr/share
	rm -rf $(INSTALLDIR)/lvm2/usr/include
