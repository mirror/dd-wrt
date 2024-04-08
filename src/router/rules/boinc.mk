boinc-configure: zlib openssl curl
	cd boinc && rm -f config.cache
	cd boinc && libtoolize
	cd boinc && aclocal
	cd boinc && autoconf
	cd boinc && autoheader
	cd boinc && autoreconf -vfi
	cd boinc && ./configure \
	--host=$(ARCH)-linux --prefix=/usr \
	--libdir=/usr/lib \
	--disable-server \
	--disable-manager \
	--enable-client \
	--enable-libraries \
	--with-opencl=no \
	--disable-boinczip \
	--enable-install-headers \
	--enable-dynamic-client-linkage \
	--with-boinc-platform=$(ARCH)-linux-uclibc \
	--with-boinc-alt-platform=$(ARCH)-generic-router-openwrt \
	CFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-L$(TOP)/curl/build/lib/.libs -lcurl -L$(TOP)/openssl -lcrypto -lssl -L$(TOP)/zlib -lz -ffunction-sections -fdata-sections -Wl,--gc-sections"
	$(MAKE) -C boinc/lib

	cd boinc && ./configure \
	--host=$(ARCH)-linux --prefix=/usr \
	--libdir=/usr/lib \
	--disable-server \
	--disable-manager \
	--enable-client \
	--enable-libraries \
	--disable-boinczip \
	--with-opencl=no \
	--enable-install-headers \
	--enable-dynamic-client-linkage \
	--with-boinc-platform=$(ARCH)-linux-uclibc \
	--with-boinc-alt-platform=$(ARCH)-generic-router-openwrt \
	CFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/zlib/include -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-L$(TOP)/curl/build/lib/.libs -lcurl -L$(TOP)/openssl -L$(TOP)/boinc/lib/.libs/ -lboinc -lcrypto -lssl -L$(TOP)/zlib -lz -ffunction-sections -fdata-sections -Wl,--gc-sections"

boinc: zlib openssl curl
	$(MAKE) -C boinc/lib
	$(MAKE) -C boinc

boinc-clean:
	$(MAKE) -C boinc clean

boinc-install:
	$(MAKE) -C boinc install DESTDIR=$(INSTALLDIR)/boinc
	install -d $(INSTALLDIR)/boinc/usr/share/boinc
	$(CP) $(TOP)/boinc/owrt/global_prefs_override.xml $(INSTALLDIR)/boinc/usr/share/boinc/global_prefs_override.xml
	$(CP) $(TOP)/boinc/owrt/remote_hosts.cfg $(INSTALLDIR)/boinc/usr/share/boinc/remote_hosts.cfg


#	$(MAKE) -C boinc install DESTDIR=$(INSTALLDIR)/boinc
	rm -rf $(INSTALLDIR)/boinc/usr/include
	rm -f $(INSTALLDIR)/boinc/usr/bin/boinc
	cd $(INSTALLDIR)/boinc/usr/bin && ln -s boinc_client boinc
#	rm -rf $(INSTALLDIR)/boinc/usr/bin
#	rm -rf $(INSTALLDIR)/boinc/usr/share
	rm -rf $(INSTALLDIR)/boinc/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/boinc/usr/lib/systemd
	rm -f $(INSTALLDIR)/boinc/usr/lib/*.a
	rm -f $(INSTALLDIR)/boinc/usr/lib/*.la