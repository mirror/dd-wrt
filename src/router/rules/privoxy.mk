
privoxy:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C privoxy

privoxy-install:
	install -D privoxy/privoxy $(INSTALLDIR)/privoxy/usr/sbin/privoxy
	install -D privoxy/match-all.action $(INSTALLDIR)/privoxy/etc/privoxy/match-all.action
	install -D privoxy/default.action $(INSTALLDIR)/privoxy/etc/privoxy/default.action
	install -D privoxy/user.action $(INSTALLDIR)/privoxy/etc/privoxy/user.action
	install -D privoxy/default.filter $(INSTALLDIR)/privoxy/etc/privoxy/default.filter
	install -D privoxy/trust $(INSTALLDIR)/privoxy/etc/privoxy/trust
	install -D privoxy/configs/privoxy.webproxy $(INSTALLDIR)/privoxy/etc/config/privoxy.webproxy
	install -D privoxy/configs/privoxy.nvramconfig $(INSTALLDIR)/privoxy/etc/config/privoxy.nvramconfig
	cp -rf privoxy/templates $(INSTALLDIR)/privoxy/etc/privoxy/
	$(STRIP) $(INSTALLDIR)/privoxy/usr/sbin/privoxy

privoxy-clean:
	$(MAKE) -C privoxy clean

privoxy-configure:
	cd privoxy && rm -rf config.{cache,status} \
	&& autoheader && autoconf \
	&& ./configure ac_cv_func_setpgrp_void=yes --prefix=/usr --sysconfdir=/etc/privoxy --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-uclibc-gcc LDFLAGS="$(COPTS) -L$(TOP)/zlib -lz -fPIC" 