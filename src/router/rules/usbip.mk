usbip-configure: util-linux
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	cd usbip/eudev && ./configure --host=$(ARCH)-linux CC="$(CC)" BLKID_CFLAGS=" " BLKID_LIBS="-lblkid -luuid" CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include  -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib -DNEED_PRINTF -D_GNU_SOURCE -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc"  --disable-nls --prefix=/usr --disable-hwdb --disable-introspection --disable-manpages --disable-selinux --enable-blkid --disable-kmod
	touch usbip/eudev/*
	make -C usbip/eudev
	cd usbip && ./autogen.sh
	cd usbip && ./configure --disable-static --enable-shared --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib  -DNEED_PRINTF -D_GNU_SOURCE -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/usbip/eudev/src/libudev -L$(TOP)/usbip/eudev/src/libudev/.libs"

usbip:
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	$(MAKE) -C usbip 

usbip-clean:
	if test -e "usbip/eudev/Makefile"; then make -C usbip/eudev clean; fi
	if test -e "usbip/Makefile"; then make -C usbip clean; fi
	@true

usbip-install:
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	make -C usbip/eudev install DESTDIR=$(INSTALLDIR)/usbip
	-mv $(INSTALLDIR)/usbip/usr/lib64 $(INSTALLDIR)/usbip/usr/lib
	rm -rf $(INSTALLDIR)/usbip/usr/bin
	rm -rf $(INSTALLDIR)/usbip/usr/etc
	rm -rf $(INSTALLDIR)/usbip/usr/include
	rm -f $(INSTALLDIR)/usbip/usr/lib/*.a
	rm -f $(INSTALLDIR)/usbip/usr/lib/*.la
	rm -rf $(INSTALLDIR)/usbip/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/usbip/usr/lib/udev
	rm -rf $(INSTALLDIR)/usbip/usr/sbin
	rm -rf $(INSTALLDIR)/usbip/usr/share/pkgconfig
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	install -D usbip/src/.libs/usbip $(INSTALLDIR)/usbip/usr/sbin/usbip
	install -D usbip/src/.libs/usbipd $(INSTALLDIR)/usbip/usr/sbin/usbipd
	install -D usbip/libsrc/.libs/libusbip.so.0 $(INSTALLDIR)/usbip/usr/lib/libusbip.so.0
	install -D usbip/usb.ids $(INSTALLDIR)/usbip/usr/share/hwdata/usb.ids
