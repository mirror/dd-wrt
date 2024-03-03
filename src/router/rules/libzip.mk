PKG_INSTALL:=1

MAKE_FLAGS+=VERBOSE=0

LIBZIP_PKG_BUILD_DIR=$(TOP)/libzip
LIBZIP_CMAKE_OPTIONS=-DZLIB_LIBRARY=$(TOP)/zlib/libz.so \
		    -DZLIB_INCLUDE_DIR=$(TOP)/zlib/include \
		    -DENABLE_GNUTLS=OFF \
		    -DENABLE_OPENSSL=ON \
		    -DENABLE_COMMONCRYPTO=OFF \
		    -DOPENSSL_CRYPTO_LIBRARY=$(TOP)/openssl/libcrypto.so \
		    -DOPENSSL_SSL_LIBRARY=$(TOP)/openssl/libssl.so \
		    -DOPENSSL_INCLUDE_DIR=$(TOP)/openssl/include

LIBZIP_STAGING_DIR=$(TOP)/_staging/usr
LIBZIP_EXTRA_CFLAGS=-I$(TOP)/_staging/usr/include $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include -I$(TOP)/openssl/include
LIBZIP_EXTRA_LDFLAGS=-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -L$(TOP)/openssl -lz -lssl -lcrypto


libzip-configure: zlib openssl
	$(call CMakeClean,$(LIBZIP_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(LIBZIP_PKG_BUILD_DIR),$(LIBZIP_STAGING_DIR),$(LIBZIP_CMAKE_OPTIONS),$(LIBZIP_EXTRA_CFLAGS),$(LIBZIP_EXTRA_LDFLAGS),.) 

libzip: zlib openssl
	$(MAKE) -C libzip

libzip-install:
	install -D libzip/lib/libzip.so.5.0 $(INSTALLDIR)/libzip/usr/lib/libzip.so.5.0
	cd $(INSTALLDIR)/libzip/usr/lib ; ln -s libzip.so.5.0 libzip.so.5  ; true
	cd $(INSTALLDIR)/libzip/usr/lib ; ln -s libzip.so.5.0 libzip.so  ; true

libzip-clean:
	if [ -e "$(LIBZIP_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C libzip clean ; fi