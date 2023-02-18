usbip-configure: util-linux libudev-configure
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
	cd usbip && ./autogen.sh
	cd usbip && ./configure --disable-static --enable-shared --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -fcommon -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib  -DNEED_PRINTF -D_GNU_SOURCE -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/libudev/src/libudev -L$(TOP)/libudev/src/libudev/.libs"

usbip: libudev
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	$(MAKE) -C usbip 

usbip-clean:
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
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -f $(INSTALLDIR)/util-linux/usr/lib/*.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/*.la
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	install -D usbip/src/.libs/usbip $(INSTALLDIR)/usbip/usr/sbin/usbip
	install -D usbip/src/.libs/usbipd $(INSTALLDIR)/usbip/usr/sbin/usbipd
	install -D usbip/libsrc/.libs/libusbip.so.0 $(INSTALLDIR)/usbip/usr/lib/libusbip.so.0
	install -D usbip/usb.ids $(INSTALLDIR)/usbip/usr/share/hwdata/usb.ids
