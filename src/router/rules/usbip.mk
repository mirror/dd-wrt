usbip-configure:
	cd usbip/libsysfs && ./configure --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/libsysfs clean all
	cd usbip && ./configure --disable-static --enable-shared --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/usbip/libsysfs/include" LDFLAGS="-L$(TOP)/usbip/libsysfs/lib/.libs" PACKAGE_CFLAGS="-I$(TOP)/glib20/libglib" PACKAGE_LIBS="-L$(TOP)/glib20/libglib/glib/.libs"

usbip:
	$(MAKE) -C usbip CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/usbip/libsysfs/include -I$(TOP)/glib20/libglib/glib" LDFLAGS="-L$(TOP)/usbip/libsysfs/lib/.libs -L$(TOP)/glib20/libglib/glib/.libs -lglib-2.0"

usbip-clean:
	if test -e "usbip/Makefile"; then make -C usbip clean; fi
	@true

usbip-install:
#	install -D usbip/src/.libs/usbip_bind_driver $(INSTALLDIR)/usbip/usr/sbin/usbip_bind_driver
	install -D usbip/src/.libs/usbip $(INSTALLDIR)/usbip/usr/sbin/usbip
	install -D usbip/src/.libs/usbipd $(INSTALLDIR)/usbip/usr/sbin/usbipd
	install -D usbip/libsrc/.libs/libusbip.so.0 $(INSTALLDIR)/usbip/usr/lib/libusbip.so.0
	install -D usbip/libsysfs/lib/.libs/libsysfs.so.2 $(INSTALLDIR)/usbip/usr/lib/libsysfs.so.2
	install -D usbip/usb.ids $(INSTALLDIR)/usbip/usr/share/hwdata/usb.ids
