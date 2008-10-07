wifidog-configure:
	cd wifidog && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections"

wifidog:
	$(MAKE) -j 4 -C wifidog

wifidog-clean:
	if test -e "wifidog/Makefile"; then make -C wifidog clean; fi
	@true

wifidog-install:
	install -D wifidog/src/wdctl $(INSTALLDIR)/wifidog/usr/sbin/wdctl
	install -D wifidog/src/.libs/wifidog $(INSTALLDIR)/wifidog/usr/sbin/wifidog
	install -D wifidog/libhttpd/.libs/libhttpd.so.0.0.0 $(INSTALLDIR)/wifidog/usr/lib/libhttpd.so.0
	mkdir -p $(INSTALLDIR)/wifidog/etc/config
	install -D wifidog/config/*.nvramconfig $(INSTALLDIR)/wifidog/etc/config
	install -D wifidog/config/*.webhotspot $(INSTALLDIR)/wifidog/etc/config

