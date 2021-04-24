wifidog-configure: wolfssl-configure wolfssl
	make -C wolfssl/minimal
	cd wifidog && ./autogen.sh
	mkdir -p wifidog/nossl
	mkdir -p wifidog/ssl
	cd wifidog/ssl && ../configure --disable-nls --enable-wolfssl --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) $(LDLTO) -I$(TOP)/wolfssl -I$(TOP)/wolfssl/minimal -I$(TOP)/wolfssl/standard/wolfssl  -I$(TOP)/wolfssl/wolfssl -L$(TOP)/wolfssl/minimal/src/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd wifidog/nossl && ../configure --disable-nls --disable-wolfssl --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

wifidog: wolfssl
	install -D wifidog/config/*.webhotspot httpd/ej_temp/
ifeq ($(CONFIG_TIEXTRA2),y)
	install -D private/telkom/mwifidog.webhotspot httpd/ej_temp/5wifidogm.webhotspot
endif
ifeq ($(CONFIG_OPENSSL),y)
	make -C wolfssl/minimal
	$(MAKE) -j 4 -C wifidog/ssl
else
	$(MAKE) -j 4 -C wifidog/nossl
endif

wifidog-clean:
	if test -e "wifidog/ssl/Makefile"; then make -C wifidog/ssl clean; fi
	if test -e "wifidog/nossl/Makefile"; then make -C wifidog/nossl clean; fi
	@true

wifidog-install:
	mkdir -p $(INSTALLDIR)/wifidog/etc/config
ifeq ($(CONFIG_OPENSSL),y)
	install -D wifidog/ssl/src/wdctl $(INSTALLDIR)/wifidog/usr/sbin/wdctl
	install -D wifidog/ssl/src/wifidog $(INSTALLDIR)/wifidog/usr/sbin/wifidog
	install -D wifidog/ssl/wifidog-msg.html $(INSTALLDIR)/wifidog/etc
else
	install -D wifidog/nossl/src/wdctl $(INSTALLDIR)/wifidog/usr/sbin/wdctl
	install -D wifidog/nossl/src/wifidog $(INSTALLDIR)/wifidog/usr/sbin/wifidog
	install -D wifidog/nossl/wifidog-msg.html $(INSTALLDIR)/wifidog/etc
endif

	install -D wifidog/config/*.nvramconfig $(INSTALLDIR)/wifidog/etc/config
	install -D wifidog/config/*.webhotspot $(INSTALLDIR)/wifidog/etc/config
ifeq ($(CONFIG_TIEXTRA2),y)
	install -D private/telkom/mwifidog.webhotspot $(INSTALLDIR)/wifidog/etc/config/5wifidogm.webhotspot
endif

