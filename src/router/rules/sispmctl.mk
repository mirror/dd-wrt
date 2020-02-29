sispmctl-configure: comgt
	cd sispmctl && aclocal
	cd sispmctl && automake
	cd sispmctl && ./configure --prefix=/usr --disable-shared --enable-static --host=$(ARCH)-linux CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/usb_modeswitch/libusb-compat/libusb -L$(TOP)/usb_modeswitch/libusb-compat/libusb/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF  -Drpl_malloc=malloc" \
		LIBUSB_CFLAGS="-I$(TOP)/usb_modeswitch/libusb-compat/libusb" \
		LIBUSB_LIBS="-L$(TOP)/usb_modeswitch/libusb-compat/libusb/.libs -lusb"

sispmctl:
	$(MAKE) -C sispmctl

sispmctl-clean:
	$(MAKE) -C sispmctl clean

sispmctl-install:
	$(MAKE) -C sispmctl install DESTDIR=$(INSTALLDIR)/sispmctl
	rm -f $(INSTALLDIR)/sispmctl/usr/lib/*.a
	rm -f $(INSTALLDIR)/sispmctl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/sispmctl/usr/share/man
#	install -D usb_modeswitch/libusb-compat/libusb/.libs/libusb.so $(INSTALLDIR)/sispmctl/usr/lib/libusb.so
	install -D usb_modeswitch/libusb-compat/libusb/.libs/libusb-0.1.so.4 $(INSTALLDIR)/sispmctl/usr/lib/libusb-0.1.so.4
#	install -D usb_modeswitch/libusb-compat/libusb/.libs/libusb-0.1.so.4.4.4 $(INSTALLDIR)/sispmctl/usr/lib/libusb-0.1.so.4.4.4
