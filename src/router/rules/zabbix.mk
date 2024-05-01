
zabbix: zlib
	install -D zabbix/config/zabbix.webservices httpd/ej_temp/zabbix.webservices
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C zabbix

zabbix-install:
	install -D zabbix/config/zabbix.nvramconfig $(INSTALLDIR)/zabbix/etc/config/zabbix.nvramconfig
	install -D zabbix/config/zabbix.startup $(INSTALLDIR)/zabbix/etc/config/zabbix.startup
	install -D zabbix/config/zabbix.webservices $(INSTALLDIR)/zabbix/etc/config/zabbix.webservices
	install -D zabbix/config/zbx_template.xml $(INSTALLDIR)/zabbix/etc/zabbix_template.xml
ifeq ($(CONFIG_MADWIFI),y)
	install -D zabbix/scripts/wclients.ath $(INSTALLDIR)/zabbix/usr/sbin/wclients
	install -D zabbix/scripts/clients.ath $(INSTALLDIR)/zabbix/usr/sbin/clients
else
	install -D zabbix/scripts/wclients.brcm $(INSTALLDIR)/zabbix/usr/sbin/wclients
	install -D zabbix/scripts/clients.brcm $(INSTALLDIR)/zabbix/usr/sbin/clients
endif
	install -D zabbix/scripts/topcpu $(INSTALLDIR)/zabbix/usr/sbin/topcpu
	install -D zabbix/scripts/temps $(INSTALLDIR)/zabbix/usr/sbin/temps
	install -D zabbix/src/zabbix_agent/zabbix_agentd $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd
	$(STRIP) $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd

zabbix-clean:
	$(MAKE) -C zabbix clean

zabbix-configure: zlib pcre
	cd zabbix && rm -rf config.{cache,status} \
	&& libtoolize -f -c && autoreconf --force --install \
	&& ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-uclibc-gcc \
	--disable-server --disable-proxy --disable-java --enable-agent --without-iconv \
	--with-libpcre="$(TOP)/pcre" \
	--with-libpcre-include="$(TOP)/pcre" \
	--with-libpcre-lib="$(TOP)/pcre/.libs" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DOLD_LIBC_MODE -L$(TOP)/pcre/.libs  -ffunction-sections -fdata-sections -Wl,--gc-section" \
	LIBPCRE_CFLAGS="-I$(TOP)/pcre" \
	LIBPCRE_LDFLAGS="-L$(TOP)/pcre/.libs -lpcre"
	cd zabbix && touch *
