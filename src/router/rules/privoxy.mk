
privoxy: pcre zlib
	install -D privoxy/configs/privoxy.webproxy httpd/ej_temp/privoxy.webproxy

privoxy-install:
ifeq ($(CONFIG_WOLFSSL),y)
	install -D privoxy/wolfssl/privoxy $(INSTALLDIR)/privoxy/usr/sbin/privoxy
else
	install -D privoxy/openssl/privoxy $(INSTALLDIR)/privoxy/usr/sbin/privoxy
endif
	install -D privoxy/match-all.action $(INSTALLDIR)/privoxy/etc/privoxy/match-all.action
	install -D privoxy/default.action $(INSTALLDIR)/privoxy/etc/privoxy/default.action
	install -D privoxy/user.action $(INSTALLDIR)/privoxy/etc/privoxy/user.action
	install -D privoxy/default.filter $(INSTALLDIR)/privoxy/etc/privoxy/default.filter
	install -D privoxy/user.filter $(INSTALLDIR)/privoxy/etc/privoxy/user.filter
	install -D privoxy/trust $(INSTALLDIR)/privoxy/etc/privoxy/trust
	install -D privoxy/configs/privoxy.webproxy $(INSTALLDIR)/privoxy/etc/config/privoxy.webproxy
	install -D privoxy/configs/privoxy.nvramconfig $(INSTALLDIR)/privoxy/etc/config/privoxy.nvramconfig
	cp -rf privoxy/templates $(INSTALLDIR)/privoxy/etc/privoxy/
	$(STRIP) $(INSTALLDIR)/privoxy/usr/sbin/privoxy

privoxy-clean:
	@true

privoxy-configure: pcre-configure pcre zlib
	-$(MAKE) -C privoxy clean
	mkdir -p privoxy/openssl
	mkdir -p privoxy/wolfssl
	cd privoxy && autoheader && autoconf
	cp -f privoxy/config.h.in privoxy/openssl
	cp -f privoxy/config.h.in privoxy/wolfssl
	cd privoxy && rm -rf config.{cache,status} \
	&& ./configure ac_cv_func_setpgrp_void=yes \
		--prefix=/usr \
		--enable-compression \
		--sysconfdir=/etc/privoxy \
		--target=$(ARCH)-linux \
		--host=$(ARCH) \
		--with-openssl \
		CC=$(ARCH)-linux-uclibc-gcc \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(SSLPATH)/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(SSLPATH)/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(SSLPATH) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/zlib -L$(TOP)/pcre/.libs -lz -fPIC"
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(SSLPATH)/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(SSLPATH)/include -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(SSLPATH) -L$(TOP)/zlib -L$(TOP)/pcre/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -lz -fPIC" \
	$(MAKE) -C privoxy
	install -D privoxy/privoxy privoxy/openssl


	cd privoxy && rm -rf config.{cache,status} \
	&& ./configure ac_cv_func_setpgrp_void=yes \
		--prefix=/usr \
		--enable-compression \
		--sysconfdir=/etc/privoxy \
		--target=$(ARCH)-linux \
		--host=$(ARCH) \
		--with-wolfssl \
		CC=$(ARCH)-linux-uclibc-gcc \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(WOLFSSL_SSLPATH)/standard/src/.libs -lwolfssl -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/zlib -L$(TOP)/pcre/.libs -lz -fPIC"
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(WOLFSSL_SSLPATH)/standard/src/.libs -lwolfssl -L$(TOP)/zlib -L$(TOP)/pcre/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -lz -fPIC" \
	$(MAKE) -C privoxy
	$(MAKE) -C privoxy
	install -D privoxy/privoxy privoxy/wolfssl
