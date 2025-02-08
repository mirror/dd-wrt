libevent:
	$(MAKE) -C libevent/openssl
	$(MAKE) -C libevent/wolfssl

libevent-install:
ifneq ($(CONFIG_WOLFSSL),y)
	$(MAKE) -C libevent/openssl install DESTDIR=$(INSTALLDIR)/libevent
else
	$(MAKE) -C libevent/wolfssl install DESTDIR=$(INSTALLDIR)/libevent
endif
	rm -rf $(INSTALLDIR)/libevent/usr/bin
	rm -rf $(INSTALLDIR)/libevent/usr/include
	rm -rf $(INSTALLDIR)/libevent/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/libevent/usr/lib/*.a
	rm -f $(INSTALLDIR)/libevent/usr/lib/*.la


libevent-clean:
	$(MAKE) -C libevent clean

libevent-configure: openssl
	cd libevent && ./autogen.sh
	mkdir -p libevent/wolfssl
	mkdir -p libevent/openssl
	
	cd libevent/openssl && ../configure  --disable-debug-mode --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(SSLPATH) -lssl -lcrypto"
	$(MAKE) -C libevent/openssl


	cd libevent/wolfssl && ../configure  --disable-debug-mode --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DWOLFSSL_USE_OPTIONS_H -I$(TOP)/wolfssl/standard -I$(TOP)/wolfssl/ -I$(TOP)/wolfssl/wolfssl -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DWOLFSSL_USE_OPTIONS_H -I$(TOP)/wolfssl/standard -I$(TOP)/wolfssl/ -I$(TOP)/wolfssl/wolfssl -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/wolfssl/standard/src/.libs -lwolfssl" \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	OPENSSL_LIBS="-L$(TOP)/wolfssl/standard/src/.libs -lwolfssl" \
	OPENSSL_CFLAGS="-DWOLFSSL_USE_OPTIONS_H -I$(TOP)/wolfssl/standard -I$(TOP)/wolfssl/ -I$(TOP)/wolfssl/wolfssl"
	$(MAKE) -C libevent/wolfssl
