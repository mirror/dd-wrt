nut-configure:
	cd nut && rm -f config.cache
	cd nut && libtoolize
	cd nut && aclocal
	cd nut && autoconf
	cd nut && autoheader
	cd nut && autoreconf -vfi
	cd nut && ./configure --disable-nls --disable-static --enable-shared --prefix=/usr --host=$(ARCH)-linux --with-usb \
	--with-usb-includes="-I$(TOP)/usb_modeswitch/libusb/libusb" \
	--with-usb-libs="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a" \
	--libdir=/usr/lib \
	--without-python \
	--without-python2 \
	--without-python3 \
	CC="$(CC)" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LIBUSB_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb" \
	LIBUSB_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a"

nut: pciutils
	$(MAKE) -C nut

nut-clean:
	if test -e "nut/Makefile"; then make -C nut clean; fi
	@true

nut-install:
	$(MAKE) -C nut install DESTDIR=$(INSTALLDIR)/nut
	rm -rf $(INSTALLDIR)/nut/usr/share
	rm -rf $(INSTALLDIR)/nut/usr/local
	rm -f $(INSTALLDIR)/nut/usr/lib/*.la
	rm -rf $(INSTALLDIR)/nut/usr/lib/systemd
	rm -rf $(INSTALLDIR)/nut/usr/lib/tmpfiles.d
