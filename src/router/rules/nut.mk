nut-configure: comgt
	cd nut && rm -f config.cache
	cd nut && libtoolize
	cd nut && aclocal
	cd nut && autoconf
	cd nut && autoheader
	cd nut && autoreconf -vfi
	cd nut && ./configure --disable-nls --disable-static --enable-shared --prefix=/usr --host=$(ARCH)-linux --with-usb \
	--with-usb-includes="-I$(TOP)/usb_modeswitch/libusb/libusb" \
	--with-usb-libs="-L$(TOP)/usb_modeswitch/libusb/libusb/.libs -lusb-1.0" \
	--libdir=/usr/lib \
	--without-python \
	--without-python2 \
	--without-python3 \
	--with-openssl \
	--with-openssl-includes="-I$(SSLPATH)/include" \
	--with-openssl-libs="-L$(SSLPATH) -lcrypto -lssl" \
	CC="$(CC)" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LIBUSB_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb" \
	LIBUSB_LIBS="-L$(TOP)/usb_modeswitch/libusb/libusb/.libs/ -lusb-1.0"

nut: comgt
	$(MAKE) -C nut

nut-clean:
	if test -e "nut/Makefile"; then make -C nut clean; fi
	@true

nut-install:
	$(MAKE) -C usb_modeswitch/libusb install DESTDIR=$(INSTALLDIR)/nut
	$(MAKE) -C nut install DESTDIR=$(INSTALLDIR)/nut
	rm -rf $(INSTALLDIR)/nut/usr/share
	rm -rf $(INSTALLDIR)/nut/usr/local
	rm -f $(INSTALLDIR)/nut/usr/lib/*.la
	rm -f $(INSTALLDIR)/nut/usr/lib/*.a
	rm -rf $(INSTALLDIR)/nut/usr/include
	rm -rf $(INSTALLDIR)/nut/usr/lib/systemd
	rm -rf $(INSTALLDIR)/nut/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/nut/usr/lib/tmpfiles.d
