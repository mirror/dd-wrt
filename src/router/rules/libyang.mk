PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

LIBYANG_PKG_BUILD_DIR=$(TOP)/libyang
LIBYANG_CMAKE_OPTIONS=-DPCRE_LIBRARY=$(TOP)/pcre/.libs \
		    -DPCRE_INCLUDE_DIR=$(TOP)/pcre \
		    -DENABLE_LYD_PRIV=ON \
		    -DCMAKE_BUILD_TYPE=release

LIBYANG_STAGING_DIR=$(TOP)/_staging/usr
LIBYANG_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT)
LIBYANG_EXTRA_LDFLAGS=-L$(TOP)/pcre/.libs -lpcre


libyang-configure: gzip openssl
	$(call CMakeClean,$(LIBYANG_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(LIBYANG_PKG_BUILD_DIR),$(LIBYANG_STAGING_DIR),$(LIBYANG_CMAKE_OPTIONS),$(LIBYANG_EXTRA_CFLAGS),$(LIBYANG_EXTRA_LDFLAGS)) 

libyang:
	$(MAKE) -C libyang
	-mkdir $(TOP)/libyang/src/libyang
	-cp $(TOP)/libyang/src/libyang.h $(TOP)/libyang/src/libyang
	-cp $(TOP)/libyang/src/user_types.h $(TOP)/libyang/src/libyang

libyang-install:
	rm -rf $(INSTALLDIR)/libyang/usr/lib
	install -D libyang/libyang.so.0.16.105 $(INSTALLDIR)/libyang/usr/lib/libyang.so.0.16.105
	-cd $(INSTALLDIR)/libyang/usr/lib ; ln -s libyang.so.0.16.105 libyang.so.0.16  ; true
	-cd $(INSTALLDIR)/libyang/usr/lib ; ln -s libyang.so.0.16 libyang.so  ; true

libyang-clean:
	if [ -e "$(LIBYANG_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libyang clean ; fi