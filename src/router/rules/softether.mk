PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

SOFTETHER_PKG_BUILD_DIR=$(TOP)/softether
SOFTETHER_CMAKE_OPTIONS=-DCURSES_LIBRARY=$(TOP)/ncurses/lib \
		    -DCURSES_INCLUDE_PATH=$(TOP)/ncurses/include \
		    -DOPENSSL_CRYPTO_LIBRARY=$(TOP)/openssl/libcrypto.so \
		    -DOPENSSL_SSL_LIBRARY=$(TOP)/openssl/libssl.so \
		    -DOPENSSL_INCLUDE_DIR=$(TOP)/openssl/include \
		    -DZLIB_LIBRARY=$(TOP)/zlib/libz.so \
		    -DZLIB_INCLUDE_DIR=$(TOP)/zlib/include \
		    -DLIB_READLINE=$(TOP)/readline/shlib \
		    -DCMAKE_BUILD_TYPE=release

SOFTETHER_STAGING_DIR=$(TOP)/_staging/usr
SOFTETHER_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) -I$(TOP)
SOFTETHER_EXTRA_LDFLAGS=-L$(TOP)/openssl -lcrypto -lssl -L$(TOP)/readline/shlib


softether-configure: zlib quagg ncurses
	$(call CMakeClean,$(SOFTETHER_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(SOFTETHER_PKG_BUILD_DIR),$(SOFTETHER_STAGING_DIR),$(SOFTETHER_CMAKE_OPTIONS),$(SOFTETHER_EXTRA_CFLAGS),$(SOFTETHER_EXTRA_LDFLAGS)) 

softether: zlib quagg ncurses
	$(MAKE) -C softether

softether-install:
	$(MAKE) -C softether install DESTDIR=$(INSTALLDIR)/softether
#	rm -rf $(INSTALLDIR)/softether/usr/lib/pkgconfig
#	rm -rf $(INSTALLDIR)/softether/usr/include
#	rm -rf $(INSTALLDIR)/softether/usr/share
#	rm -rf $(INSTALLDIR)/softether/usr/bin

softether-clean:
	if [ -e "$(SOFTETHER_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C softether clean ; fi