PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

CFM_PKG_BUILD_DIR=$(TOP)/cfm
CFM_CMAKE_OPTIONS=-DCMAKE_BUILD_TYPE=release -DLibNL_INCLUDE_DIR="$(TOP)/libnl-tiny/include" -DLibNL_LIBRARY="$(TOP)/libnl-tiny/libnl-tiny.so" -DLibEV_LIBRARY="$(TOP)/mrp/ev/.libs/libev.a"

CFM_STAGING_DIR=$(TOP)/_staging/usr
CFM_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE $(LTO)
CFM_EXTRA_LDFLAGS=$(LDLTO)


cfm-configure:
	-make -C mrp/ev clean
	cd mrp/ev && . ./autogen.sh
	cd mrp/ev && ./configure --prefix=/usr --disable-shared --enable-static --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	make -C mrp/ev
	rm -f $(TOP)/cfm/CMakeCache.txt
	$(call CMakeClean,$(CFM_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(CFM_PKG_BUILD_DIR),$(CFM_STAGING_DIR),$(CFM_CMAKE_OPTIONS),$(CFM_EXTRA_CFLAGS),$(CFM_EXTRA_LDFLAGS),.) 

cfm:
	make -C cfm/ev
	$(MAKE) -C cfm

cfm-install:
	$(MAKE) -C cfm install DESTDIR=$(INSTALLDIR)/cfm
	rm -rf $(INSTALLDIR)/cfm/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/cfm/usr/include
	rm -rf $(INSTALLDIR)/cfm/usr/share
	rm -rf $(INSTALLDIR)/cfm/usr/lib

cfm-clean:
	if [ -e "$(CFM_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C cfm clean ; fi