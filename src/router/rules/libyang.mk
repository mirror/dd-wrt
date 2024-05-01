PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

LIBYANG_PKG_BUILD_DIR=$(TOP)/libyang/build
LIBYANG_CMAKE_OPTIONS=-DPCRE2_LIBRARY=$(TOP)/pcre2/.libs \
		    -DPCRE2_INCLUDE_DIR=$(TOP)/pcre2/src \
		    -DENABLE_LYD_PRIV=ON \
		    -DCMAKE_BUILD_TYPE=Release -Wno-dev

LIBYANG_STAGING_DIR=$(TOP)/_staging/usr
LIBYANG_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) $(THUMB) -D_NEED_PRINTF -I$(TOP)/pcre2/src
LIBYANG_EXTRA_LDFLAGS=-L$(TOP)/pcre2/.libs -lpcre2-8 -lpthread


libyang-configure: zlib openssl pcre2
	rm -f $(TOP)/libyang/CMakeCache.txt
	rm -f $(TOP)/libyang/build/CMakeCache.txt
	mkdir -p $(TOP)/libyang/build
	$(call CMakeClean,$(LIBYANG_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(LIBYANG_PKG_BUILD_DIR),$(LIBYANG_STAGING_DIR),$(LIBYANG_CMAKE_OPTIONS),$(LIBYANG_EXTRA_CFLAGS),$(LIBYANG_EXTRA_LDFLAGS),..) 

libyang: zlib
	$(MAKE) -C libyang/build
	-mkdir -p $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/build/src/config.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/build/src/version.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/libyang.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/tree_schema.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/tree_data.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/tree.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/xml.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/dict.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/context.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/parser_schema.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/parser_data.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/printer_schema.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/printer_data.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/log.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/in.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/out.h $(TOP)/libyang/build/libyang
	-cp $(TOP)/libyang/src/set.h $(TOP)/libyang/build/libyang
#	-cp $(TOP)/libyang/src/user_types.h $(TOP)/libyang/build/libyang

libyang-install:
	rm -rf $(INSTALLDIR)/libyang
	$(MAKE) -C libyang/build install DESTDIR=$(INSTALLDIR)/libyang
	rm -rf $(INSTALLDIR)/libyang/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libyang/usr/include
	rm -rf $(INSTALLDIR)/libyang/usr/share
	rm -rf $(INSTALLDIR)/libyang/usr/bin

libyang-clean:
	if [ -e "$(LIBYANG_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libyang/build clean ; fi