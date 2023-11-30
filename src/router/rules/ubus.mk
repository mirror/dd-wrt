UBUS_PKG_BUILD_DIR=$(TOP)/ubus
UBUS_STAGING_DIR=$(TOP)/_staging
UBUS_PKG_INSTALL:=1
UBUS_CMAKE_OPTIONS+=VERBOSE=0 -DBUILD_LUA=OFF \
		    -DCMAKE_BUILD_TYPE=release \
		    -DCMAKE_AR=${shell which $(ARCH)-linux-gcc-ar} \
		    -DCMAKE_RANLIB=${shell which $(ARCH)-linux-gcc-ranlib}


UBUS_EXTRA_CFLAGS=-I$(TOP) -I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib  $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -flto=auto -fno-fat-lto-objects
UBUS_EXTRA_LDFLAGS=-L$(TOP)/libubox/  -ffunction-sections -fdata-sections -Wl,--gc-sections -fuse-ld=bfd -flto=auto -fuse-linker-plugin

ubus-configure: 
	$(call CMakeClean,$(UBUS_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(UBUS_PKG_BUILD_DIR),$(UBUS_STAGING_DIR),$(UBUS_CMAKE_OPTIONS),$(UBUS_EXTRA_CFLAGS),$(UBUS_EXTRA_LDFLAGS),.) 

ubus: json-c libubox ubus-configure
	$(MAKE) -C ubus
	-install -D ubus/libubus.so $(STAGING_DIR)/usr/lib/libubus.so
	-cp ubus/ubusmsg.h $(STAGING_DIR)/usr/include/
	-cp ubus/ubus_common.h $(STAGING_DIR)/usr/include/
	-cp ubus/libubus.h $(STAGING_DIR)/usr/include/

ubus-install:
	install -D ubus/libubus.so $(INSTALLDIR)/ubus/usr/lib/libubus.so
	install -D ubus/ubus $(INSTALLDIR)/ubus/usr/sbin/ubus
	install -D ubus/ubusd $(INSTALLDIR)/ubus/usr/sbin/ubusd

ubus-clean:
	if [ -e "$(PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C ubus clean ; fi
	$(call CMakeClean,$(PKG_BUILD_DIR))
