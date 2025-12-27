ifeq ($(ARCH),arm)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),mips64)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),i386)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(CONFIG_REALTEK),y)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),x86_64)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),aarch64)
UNBOUND_COPTS += -DNEED_PRINTF
endif

unbound-configure:
	mkdir -p unbound/openssl
	mkdir -p unbound/wolfssl
	cd unbound && autoreconf -vfi
#	cd unbound && libtoolize
#	cd unbound && aclocal
#	cd unbound && autoconf
#	cd unbound && autoheader
#	cd unbound && automake --add-missing
	cd unbound/openssl && ../configure --disable-ecdsa \
		--disable-gost \
		--enable-allsymbols \
		--enable-tfo-client \
		--enable-tfo-server \
		--enable-subnet \
		--with-chroot-dir=/tmp \
		--with-ssl="$(SSLPATH)" \
		--with-pthreads \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(UNBOUND_COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -lcrypto -lssl"

	-cd unbound/wolfssl && ../configure --disable-ecdsa \
		--disable-gost \
		--enable-allsymbols \
		--enable-tfo-client \
		--enable-tfo-server \
		--enable-subnet \
		--with-chroot-dir=/tmp \
		--with-ssl="$(WOLFSSL_SSLPATH)/wolfssl" \
		--with-pthreads \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(UNBOUND_COPTS) -include $(TOP)/wolfssl/standard/wolfssl/options.h -DEVP_sha256=wolfSSL_EVP_sha256 -DHAVE_SESSION_TICKET -DOPENSSL_ALL -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH)/standard/wolfssl -I$(WOLFSSL_SSLPATH)/standard  -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/wolfssl -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(WOLFSSL_SSLPATH)/standard/src/.libs" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(WOLFSSL_SSLPATH)/standard/src/.libs -lwolfssl"

unbound: 
ifeq ($(CONFIG_WOLFSSL),y)
	$(MAKE) -C unbound/wolfssl
else
	$(MAKE) -C unbound/openssl
endif
unbound-clean: 
	if test -e "unbound/openssl/Makefile"; then $(MAKE) -C unbound/openssl clean ; fi
	if test -e "unbound/wolfssl/Makefile"; then $(MAKE) -C unbound/wolfssl clean ; fi

unbound-install: 
ifeq ($(CONFIG_WOLFSSL),y)
	$(MAKE) -C unbound/wolfssl install DESTDIR=$(INSTALLDIR)/unbound
else
	$(MAKE) -C unbound/openssl install DESTDIR=$(INSTALLDIR)/unbound
endif
	mkdir -p $(INSTALLDIR)/unbound/etc/unbound
	cp unbound/config/* $(INSTALLDIR)/unbound/etc/unbound
	rm -rf $(INSTALLDIR)/unbound/usr/include
	rm -rf $(INSTALLDIR)/unbound/usr/share
	rm -f $(INSTALLDIR)/unbound/usr/lib/*.a
	rm -rf $(INSTALLDIR)/unbound/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/unbound/usr/lib/*.la
#	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-checkconf
#	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control-setup
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-host
	