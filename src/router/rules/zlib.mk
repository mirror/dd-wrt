PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

ZLIB_PKG_BUILD_DIR=$(TOP)/zlib
ZLIB_CMAKE_OPTIONS=
ZLIB_STAGING_DIR=$(TOP)/_staging/usr
ZLIB_EXTRA_CFLAGS=-I$(TOP)/_staging/usr/include $(COPTS) $(MIPS16_OPT)
ZLIB_EXTRA_LDFLAGS=-L$(TOP)/_staging/usr/lib

ZLIB_CMAKE_OPTIONS=

zlib-configure:
	$(call CMakeClean,$(ZLIB_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(ZLIB_PKG_BUILD_DIR),$(ZLIB_STAGING_DIR),$(ZLIB_CMAKE_OPTIONS),$(ZLIB_EXTRA_CFLAGS),$(ZLIB_EXTRA_LDFLAGS)) 

zlib:
	$(MAKE) -C zlib
	rm -f zlib/libz.a

zlib-install:
	install -D zlib/libz.so.1.2.11 $(INSTALLDIR)/zlib/usr/lib/libz.so.1.2.11
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.11 libz.so.1  ; true
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.11 libz.so  ; true

zlib-clean:
	if [ -e "$(ZLIB_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C zlib clean ; fi
	$(call CMakeClean,$(ZLIB_PKG_BUILD_DIR))