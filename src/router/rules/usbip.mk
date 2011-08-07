usbip-configure:
	cd usbip/libsysfs && ./configure --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/libsysfs clean all
	cd usbip && ./configure --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/usbip/libsysfs/include -I$(TOP)/glib20/libglib/glib -L$(TOP)/usbip/libsysfs/lib/.libs -L$(TOP)/glib20/libglib/glib/.libs"

usbip:
	$(MAKE) -C usbip

usbip-clean:
	if test -e "usbip/Makefile"; then make -C usbip clean; fi
	@true

usbip-install:
	install -D usbip/cmd/.libs/bind_driver $(INSTALLDIR)/usbip/usr/sbin/bind_driver
	install -D usbip/cmd/.libs/usbip $(INSTALLDIR)/usbip/usr/sbin/usbip
	install -D usbip/cmd/.libs/usbipd $(INSTALLDIR)/usbip/usr/sbin/usbipd
	install -D usbip/lib/.libs/libusbip.so.0 $(INSTALLDIR)/usbip/usr/lib/libusbip.so.0
	install -D usbip/usb.ids $(INSTALLDIR)/usbip/usr/share/hwdata/usb.ids
