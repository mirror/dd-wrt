STAGING_DIR=$(TOP)/_staging
PKG_INSTALL:=1

UBOX_PKG_BUILD_DIR=$(TOP)/libubox
UBOX_CMAKE_OPTIONS=-DBUILD_LUA=off
UBOX_STAGING_DIR=$(TOP)/_staging
UBOX_EXTRA_CFLAGS=-I$(TOP)/_staging/usr/include -I$(TOP)/_staging/usr/include/json-c $(COPTS) $(MIPS16_OPT) $(THUMB)
UBOX_EXTRA_LDFLAGS=-L$(TOP)/_staging/usr/lib

MAKE_FLAGS+=VERBOSE=0

libubox-configure: 
	$(call CMakeClean,$(UBOX_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(UBOX_PKG_BUILD_DIR),$(UBOX_STAGING_DIR),$(UBOX_CMAKE_OPTIONS),$(UBOX_EXTRA_CFLAGS),$(UBOX_EXTRA_LDFLAGS),.) 

libubox: libubox-configure 
	$(MAKE) -C libubox
	-mkdir -p $(TOP)/_staging
	-mkdir -p $(STAGING_DIR)/usr/include/libubox
	-install -D libubox/libubox.so $(STAGING_DIR)/usr/lib/libubox.so
	-install -D libubox/libjson_script.so $(STAGING_DIR)/usr/lib/libjson_script.so
	-install -D libubox/libblobmsg_json.so $(STAGING_DIR)/usr/lib/libblobmsg_json.so
	-cp libubox/*.h $(STAGING_DIR)/usr/include/libubox

libubox-install:
	install -D libubox/libubox.so $(INSTALLDIR)/libubox/usr/lib/libubox.so
	install -D libubox/libblobmsg_json.so $(INSTALLDIR)/libubox/usr/lib/libblobmsg_json.so 

libubox-clean:
	if [ -e "$(UBOX_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libubox clean ; fi
