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
SOFTETHER_EXTRA_CFLAGS=$(COPTS) $(MIPS16_OPT) -I$(TOP) -I $(TOP)/openssl/include -I$(TOP)/libsodium/src/libsodium/include -lcrypto -Wno-incompatible-pointer-types
SOFTETHER_EXTRA_LDFLAGS=-L$(TOP)/openssl -lcrypto -lssl -L$(TOP)/readline/shlib -L$(TOP)/ncurses/lib -L$(TOP)/zlib -L$(TOP)/libsodium/src/libsodium/.libs -lreadline -lhistory -lncurses -lz


softether-configure: zlib readline ncurses
	rm -f $(TOP)/softether/CMakeCache.txt
	rm -rf $(TOP)/softether/host
	mkdir $(TOP)/softether/host
	-cp -urv $(TOP)/softether/* $(TOP)/softether/host
	sed -i 's/\SHARED/STATIC/g' $(TOP)/softether/host/src/Mayaqua/CMakeLists.txt
	sed -i 's/\SHARED/STATIC/g' $(TOP)/softether/host/src/Cedar/CMakeLists.txt
	sed -i 's/\readline/libreadline.a/g' $(TOP)/softether/host/src/Cedar/CMakeLists.txt
	cd $(TOP)/softether/host && export CC=gcc && export LD=ld && export CFLAGS=-O2 && cmake -DCMAKE_BUILD_TYPE=release .
	cd $(TOP)/softether/host && export CC=gcc && export LD=ld && export CFLAGS=-O2 && make
	cp $(TOP)/softether/host/src/hamcorebuilder/hamcorebuilder /usr/local/bin
	$(call CMakeClean,$(SOFTETHER_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(SOFTETHER_PKG_BUILD_DIR),$(SOFTETHER_STAGING_DIR),$(SOFTETHER_CMAKE_OPTIONS),$(SOFTETHER_EXTRA_CFLAGS),$(SOFTETHER_EXTRA_LDFLAGS),.) 

softether: zlib ncurses
	cp -f $(TOP)/softether/config/* httpd/ej_temp/
	$(MAKE) -C softether

softether-install:
	rm -rf $(INSTALLDIR)/softether
	mkdir -p $(INSTALLDIR)/softether/usr/lib
	cp $(TOP)/softether/libcedar.so $(INSTALLDIR)/softether/usr/lib
	cp $(TOP)/softether/libmayaqua.so $(INSTALLDIR)/softether/usr/lib
	mkdir -p $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/vpnserver $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/vpnbridge $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/vpnclient $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/hamcore.se2 $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/vpncmd $(INSTALLDIR)/softether/usr/libexec/softethervpn
	cp $(TOP)/softether/files/launcher.sh $(INSTALLDIR)/softether/usr/libexec/softethervpn
	chmod 777 $(INSTALLDIR)/softether/usr/libexec/softethervpn/launcher.sh
	cp $(TOP)/softether/files/dummy $(INSTALLDIR)/softether/usr/libexec/softethervpn/lang.config
	cp $(TOP)/softether/files/dummy $(INSTALLDIR)/softether/usr/libexec/softethervpn/vpn_server.config
	cp $(TOP)/softether/files/dummy $(INSTALLDIR)/softether/usr/libexec/softethervpn/vpn_bridge.config
	cp $(TOP)/softether/files/dummy $(INSTALLDIR)/softether/usr/libexec/softethervpn/vpn_client.config
	mkdir -p $(INSTALLDIR)/softether/usr/bin
	cd $(INSTALLDIR)/softether/usr/bin && ln -s ../../usr/libexec/softethervpn/launcher.sh vpncmd
	mkdir -p $(INSTALLDIR)/softether/etc/config
	cp -f $(TOP)/softether/config/* $(INSTALLDIR)/softether/etc/config


softether-clean:
	if [ -e "$(SOFTETHER_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C softether clean ; fi