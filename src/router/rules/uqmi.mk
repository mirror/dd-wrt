PKG_BUILD_DIR_UQMI=$(TOP)/uqmi
STAGING_DIR=$(TOP)/_staging
PKG_INSTALL:=1
CMAKE_C_COMPILER:=$(ARCH)-linux-uclibc-gcc

MAKE_FLAGS+=VERBOSE=0
EXTRA_CFLAGS:=-I$(TOP)/_staging/usr/include -L$(TOP)/_staging/usr/lib -DNEED_PRINTF
LDFLAGS+=-L$(TOP)/_staging/usr/lib


uqmi-configure: 
	$(call CMakeConfigure,$(PKG_BUILD_DIR_UQMI),$(STAGING_DIR),$(CMAKE_OPTIONS))

uqmi: 
	$(MAKE) -C uqmi

uqmi-install:
	install -D uqmi/uqmi $(INSTALLDIR)/uqmi/usr/sbin/uqmi

uqmi-clean:
	$(MAKE) -C uqmi
