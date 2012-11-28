PKG_BUILD_DIR_LIBUBOX=$(TOP)/libubox
STAGING_DIR=$(TOP)/_staging
PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=1


libubox-configure: 
	$(call CMakeConfigure,$(PKG_BUILD_DIR_LIBUBOX),$(STAGING_DIR),$(CMAKE_OPTIONS))

libubox: json-c libubox-configure
#	if [ ! -e "$(PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libubox clean ; fi
	$(MAKE) -C libubox
	-mkdir -p $(TOP)/_staging
	-mkdir -p $(STAGING_DIR)/usr/include/libubox
	-install -D libubox/libubox.so $(STAGING_DIR)/usr/lib/libubox.so
	-install -D libubox/libblobmsg_json.so $(STAGING_DIR)/usr/lib/libblobmsg_json.so
	-cp libubox/*.h $(STAGING_DIR)/usr/include/libubox
	#$(MAKE) -C libubox install

libubox-install:
	install -D libubox/libubox.so $(INSTALLDIR)/libubox/usr/lib/libubox.so
	install -D libubox/libblobmsg_json.so $(INSTALLDIR)/libubox/usr/lib/libblobmsg_json.so 

libubox-clean:
	if [ -e "$(PKG_BUILD_DIR_LIBUBOX)/Makefile" ]; then $(MAKE) -C libubox clean ; fi
	$(call CMakeClean,$(PKG_BUILD_DIR_LIBUBOX))
