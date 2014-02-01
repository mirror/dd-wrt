LIBPLIST_PKG_BUILD_DIR=$(TOP)/ipeth/libplist

ipeth-configure: 
	cd $(TOP)/ipeth/libxml2 && ./configure  --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" --without-python \
	--enable-shared \
	--enable-static \
	--with-c14n \
	--without-catalog \
	--with-debug \
	--without-docbook \
	--with-html \
	--without-ftp \
	--with-minimum \
	--without-http \
	--without-iconv \
	--without-iso8859x \
	--without-legacy \
	--with-output \
	--without-pattern \
	--without-push \
	--without-python \
	--with-reader \
	--without-readline \
	--without-regexps \
	--with-sax1 \
	--with-schemas \
	--with-threads \
	--with-tree \
	--with-valid \
	--with-writer \
	--with-xinclude \
	--with-xpath \
	--with-xptr \
	--with-zlib 

	cd $(TOP)/ipeth/libxml2 && make
	
	rm -f $(TOP)/ipeth/libplist/CMakeCache.txt
	$(call CMakeClean,$(LIBPLIST_PKG_BUILD_DIR))
	(cd  $(TOP)/ipeth/libplist; \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) -I$(TOP)/ipeth/libplist/include -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) -I$(TOP)/ipeth/libplist -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		cmake \
			-DCMAKE_SYSTEM_NAME=Linux \
			-DCMAKE_SYSTEM_VERSION=1 \
			-DCMAKE_SYSTEM_PROCESSOR=$(ARCH) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_C_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
			-DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)g++ \
			-DCMAKE_EXE_LINKER_FLAGS="$(TARGET_LDFLAGS) -lm" \
			-DCMAKE_MODULE_LINKER_FLAGS="$(TARGET_LDFLAGS) -lm" \
			-DCMAKE_SHARED_LINKER_FLAGS="$(TARGET_LDFLAGS) -lm" \
			-DCMAKE_FIND_ROOT_PATH=$(STAGING_DIR) \
			-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=$(STAGING_DIR_HOST) \
			-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=$(STAGING_DIR) \
			-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=$(STAGING_DIR) \
			-DCMAKE_STRIP=: \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DLIBXML2_INCLUDE_DIR=$(TOP)/ipeth/libxml2/include \
			-DLIBXML2_LIBRARIES=$(TOP)/ipeth/libxml2/.libs/libxml2.a \
		. \
	)

	cd $(TOP)/ipeth/libplist && make


	rm -f $(TOP)/ipeth/libusbmuxd/CMakeCache.txt
	$(call CMakeClean,$(TOP)/ipeth/libusbmuxd)
	(cd  $(TOP)/ipeth/libusbmuxd; \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) -I$(TOP)/ipeth/libusbmuxd/include  -ffunction-sections -fdata-sections -Wl,--gc-sections " \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) $(MIPS16_OPT) -I$(TOP)/ipeth/libusbmuxd  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		cmake \
			-DCMAKE_SYSTEM_NAME=Linux \
			-DCMAKE_SYSTEM_VERSION=1 \
			-DCMAKE_SYSTEM_PROCESSOR=$(ARCH) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_C_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
			-DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)g++ \
			-DCMAKE_EXE_LINKER_FLAGS="$(TARGET_LDFLAGS) -lpthread" \
			-DCMAKE_MODULE_LINKER_FLAGS="$(TARGET_LDFLAGS) -lpthread" \
			-DCMAKE_SHARED_LINKER_FLAGS="$(TARGET_LDFLAGS) -lpthread" \
			-DCMAKE_FIND_ROOT_PATH=$(STAGING_DIR) \
			-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=$(STAGING_DIR_HOST) \
			-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=$(STAGING_DIR) \
			-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=$(STAGING_DIR) \
			-DCMAKE_STRIP=: \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DUSB_INCLUDE_DIR=$(TOP)/usb_modeswitch/libusb/libusb \
			-DUSB_LIBRARY=$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a \
			-DPLIST_INCLUDE_DIR=$(TOP)/ipeth/libplist/include \
			-DPLIST_LIBRARY=$(TOP)/ipeth/libplist/src/libplist.so \
		. \
	)
	cd $(TOP)/ipeth/libusbmuxd && make

	cd $(TOP)/ipeth/libimobiledevice && ./configure --without-cython --host=$(ARCH)-linux \
		ac_cv_sys_file_offset_bits=64 \
		CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -I$(TOP)/ipeth  -Drpl_localtime=localtime -I$(TOP)/openssl/include -Drpl_malloc=malloc -Drpl_realloc=realloc" \
		LDFLAGS="-L$(TOP)/ipeth/nettle -L$(TOP)/openssl -L$(TOP)/ipeth/libusbmuxd/libusbmuxd -L$(TOP)/zlib" \
		libusbmuxd_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb -I$(TOP)/ipeth/libusbmuxd/libusbmuxd" \
		libusbmuxd_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a -lusbmuxd" \
		libplist_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplist_LIBS="-L$(TOP)/ipeth/libplist/src -lplist" \
		libplistmm_CFLAGS="-I$(TOP)/ipeth/libplist/include" \
		libplistmm_LIBS="-L$(TOP)/ipeth/libplist/src -lplist"
	cd $(TOP)/ipeth/libimobiledevice && make




ipeth: comgt
ifneq ($(CONFIG_FREERADIUS),y)
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_AIRCRACK),y)
ifneq ($(CONFIG_POUND),y)
ifneq ($(CONFIG_OPENVPN),y)
	rm -f openssl/*.so*
endif
endif
endif
endif
endif
	$(MAKE) -C $(TOP)/ipeth/libxml2
	$(MAKE) -C $(TOP)/ipeth/libplist
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd 
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair clean
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair
	
ipeth-clean:
	$(MAKE) -C $(TOP)/ipeth/libxml2 clean
	$(MAKE) -C $(TOP)/ipeth/libplist clean
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd clean
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice clean
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair clean

ipeth-install:
	install -D $(TOP)/ipeth/libplist/src/libplist.so.1 $(INSTALLDIR)/ipeth/usr/lib/libplist.so.1
#	install -D $(TOP)/ipeth/libusbmuxd/libusbmuxd/libusbmuxd.so.2 $(INSTALLDIR)/ipeth/usr/lib/libusbmuxd.so.2
	install -D $(TOP)/ipeth/libusbmuxd/daemon/usbmuxd $(INSTALLDIR)/ipeth/usr/sbin/usbmuxd
#	install -D $(TOP)/ipeth/libimobiledevice/src/.libs/libimobiledevice.so.3 $(INSTALLDIR)/ipeth/usr/lib/libimobiledevice.so.3
	install -D $(TOP)/ipeth/ipheth-pair/ipheth-pair $(INSTALLDIR)/ipeth/usr/sbin/ipheth-pair
	install -D $(TOP)/ipeth/ipheth-pair/ipheth-loop $(INSTALLDIR)/ipeth/usr/sbin/ipheth-loop
	@true
