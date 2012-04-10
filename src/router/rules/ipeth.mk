
ipeth-configure: 
	cd $(TOP)/ipeth/libxml2 && ./configure  --host=$(ARCH)-linux CFLAGS="$(COPTS) -fPIC" --without-python
	cd $(TOP)/ipeth/libxml2 && make
	
	rm -f $(TOP)/ipeth/libplist/CMakeCache.txt
	(cd  $(TOP)/ipeth/libplist; \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) -I$(TOP)/ipeth/libplist/include -DNEED_PRINTF  -fPIC" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) -I$(TOP)/ipeth/libplist -DNEED_PRINTF  -fPIC" \
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
	(cd  $(TOP)/ipeth/libusbmuxd; \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) -I$(TOP)/ipeth/libusbmuxd/include -DNEED_PRINTF" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(COPTS) -I$(TOP)/ipeth/libusbmuxd -DNEED_PRINTF" \
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

	cd $(TOP)/ipeth/libimobiledevice && ./configure --without-cython --host=$(ARCH)-linux CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/ipeth  -Drpl_localtime=localtime -I$(TOP)/openssl/include -Drpl_malloc=malloc -Drpl_realloc=realloc" LDFLAGS="-L$(TOP)/ipeth/nettle -L$(TOP)/openssl -L$(TOP)/ipeth/libplist/src/ -L$(TOP)/ipeth/libusbmuxd/libusbmuxd -L$(TOP)/zlib" 
	cd $(TOP)/ipeth/libimobiledevice && make




ipeth:
	$(MAKE) -C $(TOP)/ipeth/libxml2
	$(MAKE) -C $(TOP)/ipeth/libplist
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd 
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair
	
ipeth-clean:
	$(MAKE) -C $(TOP)/ipeth/libxml2 clean
	$(MAKE) -C $(TOP)/ipeth/libplist clean
	$(MAKE) -C $(TOP)/ipeth/libusbmuxd clean
	$(MAKE) -C $(TOP)/ipeth/libimobiledevice clean
	$(MAKE) -C $(TOP)/ipeth/ipheth-pair clean

ipeth-install:
	install -D $(TOP)/ipeth/libplist/src/libplist.so.1 $(INSTALLDIR)/ipeth/usr/lib/libplist.so.1
	install -D $(TOP)/ipeth/libusbmuxd/libusbmuxd/libusbmuxd.so.2 $(INSTALLDIR)/ipeth/usr/lib/libusbmuxd.so.2
	install -D $(TOP)/ipeth/libimobiledevice/src/.libs/libimobiledevice.so.3 $(INSTALLDIR)/ipeth/usr/lib/libimobiledevice.so.3
	install -D $(TOP)/ipeth/ipheth-pair/ipheth-pair $(INSTALLDIR)/ipeth/usr/sbin/ipheth-pair
	@true
