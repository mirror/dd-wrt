TRANSMISSION_PKG_BUILD_DIR=$(TOP)/transmission
TRANSMISSION_CMAKE_OPTIONS=-DOPENSSL_CRYPTO_LIBRARY=$(SSLPATH)/libcrypto.so \
		    -DOPENSSL_SSL_LIBRARY=$(SSLPATH)/libssl.so \
		    -DOPENSSL_INCLUDE_DIR=$(SSLPATH)/include \
		    -DCURL_INCLUDE_DIR=$(TOP)/curl/include \
		    -DCURL_LIBRARY=$(TOP)/curl/build/lib/.libs/libcurl.so \
		    -DCMAKE_BUILD_TYPE=release \
		    -DCMAKE_AR=${shell which $(ARCH)-linux-gcc-ar} \
		    -DCMAKE_RANLIB=${shell which $(ARCH)-linux-gcc-ranlib}

TRANSMISSION_STAGING_DIR=$(TOP)/_staging/usr
TRANSMISSION_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -I$(TOP) -I $(SSLPATH)/include -L $(SSLPATH) -lcrypto -DNEED_PRINTF -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -ffunction-sections -fdata-sections -Wl,--gc-sections 
TRANSMISSION_EXTRA_LDFLAGS=$(LDLTO) -L$(SSLPATH) -lcrypto -lssl -L$(TOP)/ncurses/lib -L$(TOP)/zlib -lz -latomic -ffunction-sections -fdata-sections -Wl,--gc-sections


transmission: curl zlib
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

transmission-configure: curl-configure zlib
	rm -f $(TOP)/transmission/CMakeCache.txt
	rm -rf $(TOP)/transmission/build
	mkdir -p $(TOP)/transmission/build
	$(call CMakeClean,$(TRANSMISSION_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(TRANSMISSION_PKG_BUILD_DIR),$(TRANSMISSION_STAGING_DIR),$(TRANSMISSION_CMAKE_OPTIONS),$(TRANSMISSION_EXTRA_CFLAGS),$(TRANSMISSION_EXTRA_LDFLAGS),-S $(TOP)/transmission,-B $(TOP)/transmission/build) 
