LIBPLIST_PKG_BUILD_DIR=$(TOP)/ipeth/libplist

ipeth-configure: 
	cd $(TOP)/ipeth/libplist && ./autogen.sh
	cd $(TOP)/ipeth/libplist && ./configure --host=$(ARCH)-linux --without-cython CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist/include -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="-lm -ffunction-sections -fdata-sections -Wl,--gc-sections"

	cd $(TOP)/ipeth/libplist && make


	cd $(TOP)/ipeth/libusbmuxd && ./autogen.sh
	cd $(TOP)/ipeth/libusbmuxd && ./configure --host=$(ARCH)-linux --without-cython CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist/include -I$(TOP)/usb_modeswitch/libusb/libusb -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="-L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		libplist_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplist_LIBS="-L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0" ac_cv_func_malloc_0_nonnull=yes  ac_cv_func_realloc_0_nonnull=yes
		
	cd $(TOP)/ipeth/libusbmuxd && make

	cd $(TOP)/ipeth/libimobiledevice && ./autogen.sh
	cd $(TOP)/ipeth/libimobiledevice && ./configure --without-cython --host=$(ARCH)-linux \
		ac_cv_sys_file_offset_bits=64 \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -I$(TOP)/ipeth  -Drpl_localtime=localtime -I$(TOP)/openssl/include -Drpl_malloc=malloc -Drpl_realloc=realloc" \
		LDFLAGS="-L$(TOP)/ipeth/nettle -L$(TOP)/openssl -L$(TOP)/ipeth/libusbmuxd/src/.libs -lusbmuxd-2.0 -L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0  -L$(TOP)/zlib" \
		openssl_CFLAGS="-I$(TOP)/openssl/include" \
		openssl_LIBS="-L$(TOP)/openssl -lssl -lcrypto" \
		libusbmuxd_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb -I$(TOP)/ipeth/libusbmuxd/include" \
		libusbmuxd_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a -lusbmuxd-2.0" \
		libplist_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplist_LIBS="-L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0" \
		libplistmm_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplistmm_LIBS="-L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0" \
		openssl_CFLAGS="-I$(TOP)/openssl/include" \
		openssl_LIBS="-L$(TOP)/openssl -lcrypto -lssl"

	cd $(TOP)/ipeth/libimobiledevice && make

	cd $(TOP)/ipeth/usbmuxd && ./autogen.sh
	cd $(TOP)/ipeth/usbmuxd && ./configure --host=$(ARCH)-linux --without-cython CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist/include -I$(TOP)/usb_modeswitch/libusb/libusb -I$(TOP)/ipeth/libimobiledevice/include -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/ipeth/libplist -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="-L$(TOP)/openssl -lssl -lcrypto -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		libusbmuxd_CFLAGS="-I$(TOP)/ipeth/libusbmuxd/include" \
		libusbmuxd_LIBS="$(TOP)/ipeth/libusbmuxd/src/.libs/libusbmuxd-2.0.a" \
		libplist_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplist_LIBS="-L$(TOP)/ipeth/libplist/src/.libs -lplist-2.0" ac_cv_func_malloc_0_nonnull=yes  ac_cv_func_realloc_0_nonnull=yes \
		libimobiledevice_CFLAGS="-I$(TOP)/ipeth/libimobiledevice/include" \
		libimobiledevice_LIBS="$(TOP)/ipeth/libimobiledevice/src/.libs/libimobiledevice-1.0.a $(TOP)/ipeth/libusbmuxd/src/.libs/libusbmuxd-2.0.a" \
		libusb_CFLAGS="-I$(TOP)/ipeth/libusbmuxd/include -I$(TOP)/usb_modeswitch/libusb/libusb" \
		libusb_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a"

	cd $(TOP)/ipeth/usbmuxd && make


ipeth: comgt
ifneq ($(CONFIG_FREERADIUS),y)
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_AIRCRACK),y)
ifneq ($(CONFIG_POUND),y)
ifneq ($(CONFIG_OPENVPN),y)
ifneq ($(CONFIG_DDNS),y)
	rm -f openssl/*.so*
endif
endif
endif
endif
endif
endif
	$(MAKE) -C $(TOP)/ipeth/libplist
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd 
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice
	$(MAKE) -C $(TOP)/ipeth/usbmuxd 
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair clean
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair
	
ipeth-clean:
	$(MAKE) -C $(TOP)/ipeth/libplist clean
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd clean
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice clean
	$(MAKE) -C $(TOP)/ipeth/usbmuxd clean
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair clean

ipeth-install:
	install -D $(TOP)/ipeth/libplist/src/.libs/libplist-2.0.so.3 $(INSTALLDIR)/ipeth/usr/lib/libplist-2.0.so.3
#	install -D $(TOP)/ipeth/libusbmuxd/libusbmuxd/libusbmuxd.so.2 $(INSTALLDIR)/ipeth/usr/lib/libusbmuxd.so.2
	install -D $(TOP)/ipeth/usbmuxd/src/.libs/usbmuxd $(INSTALLDIR)/ipeth/usr/sbin/usbmuxd
#	install -D $(TOP)/ipeth/libimobiledevice/src/.libs/libimobiledevice.so.3 $(INSTALLDIR)/ipeth/usr/lib/libimobiledevice.so.3
	install -D $(TOP)/ipeth/ipheth-pair/ipheth-pair $(INSTALLDIR)/ipeth/usr/sbin/ipheth-pair
	install -D $(TOP)/ipeth/ipheth-pair/ipheth-loop $(INSTALLDIR)/ipeth/usr/sbin/ipheth-loop
	@true