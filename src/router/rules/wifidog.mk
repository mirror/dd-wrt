wifidog:
	cd wifidog && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS)"
	$(MAKE) -C wifidog

wifidog-clean:
	if test -e "wifidog/Makefile"; then make -C wifidog clean; fi
	@true

wifidog-install:
	install -D wifidog/src/wdctl $(INSTALLDIR)/wifidog/usr/sbin/wdctl
	install -D wifidog/src/wifidog $(INSTALLDIR)/wifidog/usr/sbin/wifidog
	mkdir -p $(INSTALLDIR)/wifidog/etc/config
	install -D wifidog/config/*.nvramconfig $(INSTALLDIR)/wifidog/etc/config
	install -D wifidog/config/*.webhotspot $(INSTALLDIR)/wifidog/etc/config

