
zabbix:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C zabbix

zabbix-install:
	install -D zabbix/config/zabbix.nvramconfig $(INSTALLDIR)/zabbix/etc/config/zabbix.nvramconfig
	install -D zabbix/config/zabbix.startup $(INSTALLDIR)/zabbix/etc/config/zabbix.startup
	install -D zabbix/config/zabbix.webservices $(INSTALLDIR)/zabbix/etc/config/zabbix.webservices
	install -D zabbix/scripts/wclients $(INSTALLDIR)/zabbix/usr/sbin/wclients
	install -D zabbix/scripts/clients $(INSTALLDIR)/zabbix/usr/sbin/clients
	install -D zabbix/scripts/topcpu $(INSTALLDIR)/zabbix/usr/sbin/topcpu
	install -D zabbix/scripts/temps $(INSTALLDIR)/zabbix/usr/sbin/temps
	install -D zabbix/src/zabbix_agent/zabbix_agentd $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd
	$(STRIP) $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd

zabbix-clean:
	$(MAKE) -C zabbix clean

zabbix-configure:
	cd zabbix && rm -rf config.{cache,status} \
	&& autoheader && autoconf \
	&& ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-uclibc-gcc \
	--disable-server --disable-proxy --disable-java --enable-agent \
	-with-iconv-include=$(TOP)/glib20/libiconv/include --with-iconv-lib=$(TOP)/glib20/libiconv/lib/.libs \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DOLD_LIBC_MODE -ffunction-sections -fdata-sections -Wl,--gc-section"
