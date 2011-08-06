usbip-configure:
	cd usbip/libiconv && ./configure --enable-static --disable-shared --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/libiconv clean all
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*.so*
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*.la*
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*la*

	cd usbip/gettext && ./configure --enable-static --disable-shared --disable-openmp --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/gettext clean all

	cd usbip/libglib && ./configure --enable-shared --disable-static --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc -I$(TOP)/usbip/gettext/gettext-runtime/intl  -I$(TOP)/usbip/libiconv/include -L$(TOP)/usbip/libiconv/lib/.libs -L$(TOP)/usbip/gettext/gettext-runtime/intl/.libs" --with-libiconv=gnu
	make -C usbip/libglib clean all
	cd usbip/libsysfs && ./configure --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/libsysfs clean all
	rm -f $(TOP)/usbip/libsysfs/lib/.libs/*.so*
	rm -f $(TOP)/usbip/libsysfs/lib/.libs/*.la*
	rm -f $(TOP)/usbip/libsysfs/lib/.libs/*la*
	cd usbip && ./configure --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/usbip/libsysfs/include -I$(TOP)/usbip/libglib/glib -L$(TOP)/usbip/libsysfs/lib/.libs -L$(TOP)/usbip/libglib/glib/.libs"

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
	install -D usbip/libglib/glib/.libs/libglib-2.0.so.0 $(INSTALLDIR)/usbip/usr/lib/libglib-2.0.so.0

