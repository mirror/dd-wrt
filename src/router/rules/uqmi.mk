PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

UQMI_PKG_BUILD_DIR=$(TOP)/uqmi
UQMI_CMAKE_OPTIONS=
UQMI_STAGING_DIR=$(TOP)/_staging/usr
UQMI_EXTRA_CFLAGS=-I$(TOP)/_staging/usr/include -DNEED_PRINTF $(MIPS16_OPT) $(LTO)
UQMI_EXTRA_LDFLAGS=-L$(TOP)/_staging/usr/lib $(LDLTO)

UQMI_CMAKE_OPTIONS=

uqmi-configure: json-c libubox
	$(call CMakeClean,$(UQMI_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(UQMI_PKG_BUILD_DIR),$(UQMI_STAGING_DIR),$(UQMI_CMAKE_OPTIONS),$(UQMI_EXTRA_CFLAGS),$(UQMI_EXTRA_LDFLAGS),.) 

uqmi:
	$(MAKE) -C uqmi

uqmi-install:
	install -D uqmi/uqmi/uqmi $(INSTALLDIR)/uqmi/usr/sbin/uqmi

uqmi-clean:
	if [ -e "$(UQMI_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C uqmi clean ; fi
