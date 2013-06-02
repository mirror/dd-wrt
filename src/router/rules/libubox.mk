PKG_BUILD_DIR_LIBUBOX=$(TOP)/libubox
STAGING_DIR=$(TOP)/_staging
PKG_INSTALL:=1


MAKE_FLAGS+=VERBOSE=0
EXTRA_CFLAGS:=-I$(TOP)/_staging/usr/include
LDFLAGS+=-L$(TOP)/_staging/usr/lib


libubox-configure: 
	-$(call CMakeConfigure,$(PKG_BUILD_DIR_LIBUBOX),$(STAGING_DIR),$(CMAKE_OPTIONS) -DBUILD_LUA=OFF CMAKE_C_COMPILER=$(ARCH)-linux-uclibc-gcc)

libubox:
	$(MAKE) -C libubox
	-mkdir -p $(TOP)/_staging
	-mkdir -p $(STAGING_DIR)/usr/include/libubox
	-install -D libubox/libubox.so $(STAGING_DIR)/usr/lib/libubox.so
#	-install -D libubox/libjson_script.so $(STAGING_DIR)/usr/lib/libjson_script.so
#	-install -D libubox/libblobmsg_json.so $(STAGING_DIR)/usr/lib/libblobmsg_json.so
	-cp libubox/*.h $(STAGING_DIR)/usr/include/libubox

libubox-install:
	install -D libubox/libubox.so $(INSTALLDIR)/libubox/usr/lib/libubox.so
#	install -D libubox/libblobmsg_json.so $(INSTALLDIR)/libubox/usr/lib/libblobmsg_json.so 

libubox-clean:
	if [ -e "$(PKG_BUILD_DIR_LIBUBOX)/Makefile" ]; then $(MAKE) -C libubox clean ; fi
	-$(call CMakeClean,$(PKG_BUILD_DIR_LIBUBOX))
