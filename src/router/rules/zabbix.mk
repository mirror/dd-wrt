
zabbix:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -Os -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -Os -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -Os -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C zabbix

zabbix-install:
	install -D zabbix/config/zabbix.nvramconfig $(INSTALLDIR)/zabbix/etc/config/zabbix.nvramconfig
	install -D zabbix/config/zabbix.startup $(INSTALLDIR)/zabbix/etc/config/zabbix.startup
	install -D zabbix/config/zabbix.webservices $(INSTALLDIR)/zabbix/etc/config/zabbix.webservices
	install -D zabbix/scripts/wclients $(INSTALLDIR)/zabbix/usr/sbin/wclients
	install -D zabbix/scripts/topcpu $(INSTALLDIR)/zabbix/usr/sbin/topcpu
	install -D zabbix/src/zabbix_agent/zabbix_agentd $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd
#	install -D zabbix/src/zabbix_agent/zabbix_agent $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agent
	$(STRIP) $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd

zabbix-clean:
	$(MAKE) -C zabbix clean

zabbix-configure:
	cd zabbix && ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-uclibc-gcc \
	--disable-server --disable-proxy --disable-java --enable-agent \
	-with-iconv-include=$(TOP)/glib20/libiconv/include --with-iconv-lib=$(TOP)/glib20/libiconv/lib/.libs \
	CFLAGS="-Os -DOLD_LIBC_MODE -ffunction-sections -fdata-sections -Wl,--gc-section"