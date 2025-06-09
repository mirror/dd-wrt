APFREE_WIFIDOG_PKG_BUILD_DIR=$(TOP)/apfree-wifidog
APFREE_WIFIDOG_STAGING_DIR=$(TOP)/_staging
APFREE_WIFIDOG_PKG_INSTALL:=1
APFREE_WIFIDOG_CMAKE_OPTIONS+=VERBOSE=0 -DBUILD_LUA=OFF -DAPFREE_WIFIDOG_LTO=OFF \
		    -DCMAKE_BUILD_TYPE=release \
		    -DCMAKE_AR=${GCCAR} \
		    -DCMAKE_RANLIB=${GCCRANLIB}


#APFREE_WIFIDOG_EXTRA_CFLAGS=-I$(TOP) -I$(STAGING_DIR)/usr/include -I$(TOP)/shared -L$(STAGING_DIR)/usr/lib -I$(TOP)/libnl-tiny/include  $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)
#APFREE_WIFIDOG_EXTRA_LDFLAGS=-ljson-c -L$(TOP)/libubox/ -L$(TOP)/libnl-tiny -lnl-tiny  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO)
APFREE_WIFIDOG_EXTRA_CFLAGS=$(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)
APFREE_WIFIDOG_EXTRA_LDFLAGS=-ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO)

apfree-wifidog-configure:
	$(call CMakeClean,$(APFREE_WIFIDOG_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(APFREE_WIFIDOG_PKG_BUILD_DIR),$(APFREE_WIFIDOG_STAGING_DIR),$(APFREE_WIFIDOG_CMAKE_OPTIONS),$(APFREE_WIFIDOG_EXTRA_CFLAGS),$(APFREE_WIFIDOG_EXTRA_LDFLAGS),.) 

apfree-wifidog:
	$(MAKE) -C apfree-wifidog

apfree-wifidog-install:
	install -D apfree-wifidog/apfree-wifidog $(INSTALLDIR)/apfree-wifidog/usr/sbin/apfree-wifidog

apfree-wifidog-clean:
	if [ -e "$(APFREE_WIFIDOG_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C apfree-wifidog clean ; fi
