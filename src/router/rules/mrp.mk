PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

MRP_PKG_BUILD_DIR=$(TOP)/mrp
MRP_CMAKE_OPTIONS=-DCMAKE_BUILD_TYPE=release -DLibNL_INCLUDE_DIR="$(TOP)/libnl-tiny/include" -DLibNL_LIBRARY="$(TOP)/libnl-tiny/libnl-tiny.so" -DLibEV_LIBRARY="$(TOP)/mrp/ev/.libs/libev.a"

MRP_STAGING_DIR=$(TOP)/_staging/usr
MRP_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE $(LTO)
MRP_EXTRA_LDFLAGS=$(LDLTO)


mrp-configure:
	-make -C mrp/ev clean
	cd mrp/ev && . ./autogen.sh
	cd mrp/ev && ./configure --prefix=/usr --disable-shared --enable-static --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	make -C mrp/ev
	rm -f $(TOP)/mrp/CMakeCache.txt
	$(call CMakeClean,$(MRP_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(MRP_PKG_BUILD_DIR),$(MRP_STAGING_DIR),$(MRP_CMAKE_OPTIONS),$(MRP_EXTRA_CFLAGS),$(MRP_EXTRA_LDFLAGS)) 

mrp:
	make -C mrp/ev
	$(MAKE) -C mrp

mrp-install:
	$(MAKE) -C mrp install DESTDIR=$(INSTALLDIR)/mrp
	rm -rf $(INSTALLDIR)/mrp/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/mrp/usr/include
	rm -rf $(INSTALLDIR)/mrp/usr/share

mrp-clean:
	if [ -e "$(MRP_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C mrp clean ; fi