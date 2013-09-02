
lighttpd:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections -lz -fPIC" \
	$(MAKE) -C lighttpd

lighttpd-install:
	install -D lighttpd/src/lighttpd $(INSTALLDIR)/lighttpd/usr/sbin/lighttpd
	mkdir -p $(INSTALLDIR)/lighttpd/usr/lib/lighttpd ; true
	mkdir -p $(INSTALLDIR)/lighttpd/etc/lighttpd ; true
	cp -av lighttpd/src/.libs/*.so $(INSTALLDIR)/lighttpd/usr/lib/lighttpd/ ; true
	cp -av $(TOP)/pcre/.libs/libpcre.so.* $(INSTALLDIR)/lighttpd/usr/lib/ ; true
#	install -D lighttpd/configs/lighttpd.nvramconfig $(INSTALLDIR)/lighttpd/etc/config/lighttpd.nvramconfig
	install -D lighttpd/configs/lighttpd.conf $(INSTALLDIR)/lighttpd/etc/lighttpd.conf
#	cp -rf privoxy/templates $(INSTALLDIR)/privoxy/etc/privoxy/
#	$(STRIP) $(INSTALLDIR)/privoxy/usr/sbin/privoxy

lighttpd-clean:
	$(MAKE) -C lighttpd

	
LIGHTTPD_CONFIGURE_ARGS+= \
        --libdir=/usr/lib/lighttpd \
        --sysconfdir=/tmp/lighttpd \
        --enable-shared \
        --enable-static \
        --without-attr \
        --without-bzip2 \
        --without-fam \
        --without-gdbm \
        --without-ldap \
        --without-lua \
        --without-memcache \
        --without-mysql \
        --with-openssl="$(TOP)/openssl" \
        --with-openssl-includes="$(TOP)/openssl/include" \
        --with-openssl-libs="$(TOP)/openssl" \
        --with-pcre \
        --without-valgrind \
        --with-zlib
	
lighttpd-configure:
	cd lighttpd && ./configure  $(LIGHTTPD_CONFIGURE_ARGS) --prefix=/usr --sysconfdir=/etc/lighttpd --target=$(ARCH)-linux --host=$(ARCH)-linux-uclibc CC=$(ARCH)-linux-uclibc-gcc CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/pcre -I$(TOP)/zlib" LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/pcre/.libs -lpthread $(LDFLAGS) -L$(TOP)/zlib -lz -fPIC" 