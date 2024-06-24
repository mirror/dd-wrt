
privoxy: pcre zlib
	install -D privoxy/configs/privoxy.webproxy httpd/ej_temp/privoxy.webproxy
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/openssl/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/openssl/include -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/pcre/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -lz -fPIC" \
	$(MAKE) -C privoxy

privoxy-install:
	install -D privoxy/privoxy $(INSTALLDIR)/privoxy/usr/sbin/privoxy
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
	$(MAKE) -C privoxy clean

privoxy-configure: pcre-configure pcre zlib
	cd privoxy && rm -rf config.{cache,status} \
	&& autoheader && autoconf \
	&& ./configure ac_cv_func_setpgrp_void=yes \
		--prefix=/usr \
		--enable-compression \
		--sysconfdir=/etc/privoxy \
		--target=$(ARCH)-linux \
		--host=$(ARCH) \
		--with-openssl \
		CC=$(ARCH)-linux-uclibc-gcc \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/openssl/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/openssl/include -I$(TOP)/zlib -I$(TOP)/pcre -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(TOP)/openssl -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/zlib -L$(TOP)/pcre/.libs -lz -fPIC"