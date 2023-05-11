TRANSMISSION_PKG_BUILD_DIR=$(TOP)/transmission
TRANSMISSION_CMAKE_OPTIONS=-DOPENSSL_CRYPTO_LIBRARY=$(TOP)/openssl/libcrypto.so \
		    -DOPENSSL_SSL_LIBRARY=$(TOP)/openssl/libssl.so \
		    -DOPENSSL_INCLUDE_DIR=$(TOP)/openssl/include \
		    -DCURL_INCLUDE_DIR=$(TOP)/curl/include \
		    -DCURL_LIBRARY=$(TOP)/curl/lib/.libs/libcurl.so \
		    -DCMAKE_BUILD_TYPE=release

TRANSMISSION_STAGING_DIR=$(TOP)/_staging/usr
TRANSMISSION_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) -I$(TOP) -I $(TOP)/openssl/include -L $(TOP)/openssl -lcrypto -DNEED_PRINTF -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 
TRANSMISSION_EXTRA_LDFLAGS=-L$(TOP)/openssl -lcrypto -lssl -L$(TOP)/ncurses/lib -L$(TOP)/zlib -lz -latomic


transmission: libevent curl zlib
	install -D transmission/configs/transmission.webnas httpd/ej_temp/06transmission.webnas
	$(MAKE) -C transmission/build

transmission-install:
	$(MAKE) -C transmission/build install DESTDIR=$(INSTALLDIR)/transmission
	mv $(INSTALLDIR)/transmission/usr/bin/transmission-daemon $(INSTALLDIR)/transmission/usr/bin/transmissiond
	rm -rf $(INSTALLDIR)/transmission/usr/share/man
	rm -f $(INSTALLDIR)/transmission/usr/lib/*.la
	rm -f $(INSTALLDIR)/transmission/usr/lib/*.a
	install -D transmission/configs/transmission.nvramconfig $(INSTALLDIR)/transmission/etc/config/transmission.nvramconfig
	install -D transmission/configs/transmission.webnas $(INSTALLDIR)/transmission/etc/config/06transmission.webnas
	mv -f $(INSTALLDIR)/transmission/usr/share/transmission/public_html $(INSTALLDIR)/transmission/usr/share/transmission/default
	cp -r transmission/combustion/ $(INSTALLDIR)/transmission/usr/share/transmission/combustion
	cp -r transmission/transmission-web-control/ $(INSTALLDIR)/transmission/usr/share/transmission/transmission-web-control

transmission-clean:
	$(MAKE) -C transmission/build clean

transmission-configure: libevent-configure curl-configure zlib
	rm -f $(TOP)/transmission/CMakeCache.txt
	rm -rf $(TOP)/transmission/build
	mkdir -p $(TOP)/transmission/build
	$(call CMakeClean,$(TRANSMISSION_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(TRANSMISSION_PKG_BUILD_DIR),$(TRANSMISSION_STAGING_DIR),$(TRANSMISSION_CMAKE_OPTIONS),$(TRANSMISSION_EXTRA_CFLAGS),$(TRANSMISSION_EXTRA_LDFLAGS),-S $(TOP)/transmission,-B $(TOP)/transmission/build) 
