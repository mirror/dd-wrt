PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

LIBYANG_PKG_BUILD_DIR=$(TOP)/libyang/build
LIBYANG_CMAKE_OPTIONS=-DPCRE_LIBRARY=$(TOP)/pcre/.libs \
		    -DPCRE_INCLUDE_DIR=$(TOP)/pcre \
		    -DENABLE_LYD_PRIV=ON \
		    -DCMAKE_BUILD_TYPE=Release -Wno-dev

LIBYANG_STAGING_DIR=$(TOP)/_staging/usr
LIBYANG_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT)
LIBYANG_EXTRA_LDFLAGS=-L$(TOP)/pcre/.libs -lpcre -lpthread


libyang-configure: zlib openssl
	rm -f $(TOP)/libyang/CMakeCache.txt
	rm -f $(TOP)/libyang/build/CMakeCache.txt
	mkdir -p $(TOP)/libyang/build
	$(call CMakeClean,$(LIBYANG_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(LIBYANG_PKG_BUILD_DIR),$(LIBYANG_STAGING_DIR),$(LIBYANG_CMAKE_OPTIONS),$(LIBYANG_EXTRA_CFLAGS),$(LIBYANG_EXTRA_LDFLAGS),..) 

libyang: zlib
	$(MAKE) -C libyang
	-mkdir $(TOP)/libyang/src/libyang
	-cp $(TOP)/libyang/src/libyang.h $(TOP)/libyang/src/libyang
	-cp $(TOP)/libyang/src/user_types.h $(TOP)/libyang/src/libyang

libyang-install:
	rm -rf $(INSTALLDIR)/libyang
	$(MAKE) -C libyang/build install DESTDIR=$(INSTALLDIR)/libyang
	rm -rf $(INSTALLDIR)/libyang/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libyang/usr/include
	rm -rf $(INSTALLDIR)/libyang/usr/share
	rm -rf $(INSTALLDIR)/libyang/usr/bin

libyang-clean:
	if [ -e "$(LIBYANG_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libyang clean ; fi