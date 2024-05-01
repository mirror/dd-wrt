DAWN_PKG_BUILD_DIR=$(TOP)/dawn
DAWN_STAGING_DIR=$(TOP)/_staging
DAWN_PKG_INSTALL:=1
DAWN_CMAKE_OPTIONS+=VERBOSE=0 -DBUILD_LUA=OFF \
		    -DCMAKE_BUILD_TYPE=release \
		    -DCMAKE_AR=${GCCAR} \
		    -DCMAKE_RANLIB=${GCCRANLIB}


DAWN_EXTRA_CFLAGS=-I$(TOP) -I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib  $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)
DAWN_EXTRA_LDFLAGS=-ljson-c -L$(TOP)/libubox/  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO)

dawn-configure: json-c libubox ubus
	$(call CMakeClean,$(DAWN_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(DAWN_PKG_BUILD_DIR),$(DAWN_STAGING_DIR),$(DAWN_CMAKE_OPTIONS),$(DAWN_EXTRA_CFLAGS),$(DAWN_EXTRA_LDFLAGS),.) 

dawn: json-c libubox ubus
	$(MAKE) -C dawn
#	-install -D dawn/libdawn.so $(STAGING_DIR)/usr/lib/libdawn.so
#	-cp dawn/dawnmsg.h $(STAGING_DIR)/usr/include/
#	-cp dawn/dawn_common.h $(STAGING_DIR)/usr/include/
#	-cp dawn/libdawn.h $(STAGING_DIR)/usr/include/

dawn-install:
#	install -D dawn/ap-monitor $(INSTALLDIR)/dawn/usr/sbin/ap-monitor
#	install -D dawn/fakeap $(INSTALLDIR)/dawn/usr/sbin/fakeap
	install -D dawn/dawnd $(INSTALLDIR)/dawn/usr/sbin/dawnd

dawn-clean:
	if [ -e "$(DAWN_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C dawn clean ; fi
