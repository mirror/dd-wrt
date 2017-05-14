libsodium:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -v -Wl,--verbose" \
	make -C libsodium
	
libsodium-clean:
	make -C libsodium clean
	
libsodium-configure:
	cd libsodium && ./configure --host=$(ARCH)-linux-uclibc  \
	--disable-ssp \
	--disable-shared \
	--enable-static \
	--enable-minimal \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)   -fPIC -v -Wl,--verbose"
	make -C libsodium


DNSCRYPT_CONFIGURE_ARGS+= \
	--disable-documentation \
	--with-include-path="$(TOP)/openssl/include $(TOP)/libsodium/src/libsodium/include" \
	--with-lib-path="$(TOP)/openssl , $(TOP)/gmp, $(TOP)/libsodium/src/libsodium/.libs"

dnscrypt-configure: openssl libsodium-configure
	cd dnscrypt && ./configure --host=$(ARCH)-linux --prefix=/usr \
	CC="ccache $(CC)" $(DNSCRYPT_CONFIGURE_ARGS) \
	CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/libsodium/src/libsodium/include/ -I$(TOP)/gmp -I$(TOP)/zlib" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/libsodium/src/libsodium/include/" \
	LDFLAGS="-L$(TOP)/libsodium/src/libsodium/.libs $(LDFLAGS)"

dnscrypt: openssl libsodium
	make -C dnscrypt 

dnscrypt-clean:
	-make -C dnscrypt clean

dnscrypt-install: 
#	mkdir -p $(INSTALLDIR)/dnscrypt/usr/lib
	mkdir -p $(INSTALLDIR)/dnscrypt/etc/dnscrypt
#	install -D libsodium/src/libsodium/.libs/libsodium.so $(INSTALLDIR)/dnscrypt/usr/lib/libsodium.so
#	cd $(INSTALLDIR)/dnscrypt/usr/lib/ && \
#		ln -sf libsodium.so libsodium.so.18
#	install -D dnscrypt/src/proxy/.libs/dnscrypt-proxy $(INSTALLDIR)/dnscrypt/usr/sbin/dnscrypt-proxy
#	install -D dnscrypt/src/hostip/.libs/hostip $(INSTALLDIR)/dnscrypt/usr/sbin/hostip
	make -C dnscrypt install DESTDIR=$(INSTALLDIR)/dnscrypt
	rm -rf $(INSTALLDIR)/dnscrypt/usr/etc
	rm -rf $(INSTALLDIR)/dnscrypt/usr/include
	rm -rf $(INSTALLDIR)/dnscrypt/usr/lib
	rm -rf $(INSTALLDIR)/dnscrypt/usr/share


	install -D dnscrypt/dnscrypt-resolvers.csv $(INSTALLDIR)/dnscrypt/etc/dnscrypt/
