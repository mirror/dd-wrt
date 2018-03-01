NSMD_PKG_BUILD_DIR=$(TOP)/nsmd
NSMD_CMAKE_OPTIONS=-DNL_CFLAGS="-D_GNU_SOURCE -I$(TOP)/libnl-tiny/include" -DNL_LIBS="-L$(TOP)/libnl-tiny/ -lnl-tiny"
STAGING_DIR=$(TOP)/_staging
#NSMD_EXTRA_CFLAGS=-v -DNEED_PRINTF -DHAVE_NL80211 -g -I$(TOP)/libnl-tiny/include -I$(TOP)/_staging/include -I$(TOP) -I$(TOP)/compat-wireless/include
#NSMD_EXTRA_LDFLAGS=-L$(TOP)/libubox -L$(TOP)/_staging/usr/lib -L$(TOP)/libnl-tiny -ljson -lubox -lblobmsg_json -lnl-tiny
NSMD_EXTRA_CFLAGS=-DNEED_PRINTF -DHAVE_NVRAM -g -I$(TOP)/nvram -I$(TOP)/_staging/usr/include -I$(TOP) -I$(TOP)/compat-wireless/include/uapi
NSMD_EXTRA_LDFLAGS=-L$(TOP)/libubox -L$(TOP)/libnl-tiny -L$(TOP)/_staging/usr/lib -L$(TOP)/nvram/ -ljson-c -lubox -lblobmsg_json -lnl-tiny -lnvram

MAKE_FLAGS+=VERBOSE=1

nsmd-configure: 
	$(call CMakeConfigure,$(NSMD_PKG_BUILD_DIR),$(STAGING_DIR),$(NSMD_CMAKE_OPTIONS),$(NSMD_EXTRA_CFLAGS),$(NSMD_EXTRA_LDFLAGS))

#nsmd-configure
nsmd: 
#	if [ ! -e "$(PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libubox clean ; fi
	$(MAKE) -C nsmd

nsmd-install:
	install -D nsmd/nsmd $(INSTALLDIR)/nsmd/usr/sbin/nsmd

nsmd-clean:
	if [ -e "$(NSMD_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C nsmd clean ; fi
	$(call CMakeClean,$(NSMD_PKG_BUILD_DIR))

