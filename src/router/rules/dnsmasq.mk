dnsmasq:
	$(MAKE) -C dnsmasq COPTS=-DHAVE_BROKEN_RTC CFLAGS="$(COPTS)"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq


