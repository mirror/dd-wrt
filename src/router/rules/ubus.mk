PKG_BUILD_DIR=$(TOP)/ubus
STAGING_DIR=$(TOP)/_staging
PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

EXTRA_CFLAGS=-I$(TOP) -I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib  $(MIPS16_OPT) -DNEED_PRINTF
EXTRA_LDFLAGS=-L$(TOP)/libubox/

ubus-configure: 
	$(call CMakeConfigure,$(PKG_BUILD_DIR),$(STAGING_DIR),$(CMAKE_OPTIONS))

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
