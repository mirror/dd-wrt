DNSCRYPT_CONFIGURE_ARGS+= \
	--disable-documentation \
	--with-include-path="$(SSLPATH)/include $(TOP)/libsodium/src/libsodium/include" \
	--with-lib-path="$(SSLPATH) , $(TOP)/gmp, $(TOP)/libsodium/src/libsodium/.libs"

dnscrypt-configure: libsodium-configure zlib
	cd dnscrypt && ./autogen.sh && \
	./configure --host=$(ARCH)-linux --prefix=/usr \
	$(DNSCRYPT_CONFIGURE_ARGS) \
	CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libsodium/src/libsodium/include/ -I$(TOP)/gmp -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -I$(TOP)/libsodium/src/libsodium/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-L$(TOP)/libsodium/src/libsodium/.libs $(LDFLAGS)  $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

dnscrypt: libsodium zlib
	make -C dnscrypt/libltdl
	make -C dnscrypt 

dnscrypt-clean:
	-make -C dnscrypt/libltdl clean
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
	rm -rf $(INSTALLDIR)/dnscrypt/usr/lib64
	rm -rf $(INSTALLDIR)/dnscrypt/usr/share
	install -D dnscrypt/dnscrypt-resolvers.csv $(INSTALLDIR)/dnscrypt/etc/dnscrypt/
#ifeq ($(CONFIG_DNSCRYPT_BLACKLIST),y)
#	install -D dnscrypt/contrib/dnscrypt-blacklist.txt $(INSTALLDIR)/dnscrypt/etc/dnscrypt/
#endif