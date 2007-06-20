dnsmasq:
	$(MAKE) -C dnsmasq COPTS=-DHAVE_BROKEN_RTC CFLAGS="$(COPTS)"
	$(MAKE) -C dnsmasq/contrib/wrt CFLAGS="$(COPTS)"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/contrib/wrt/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D dnsmasq/contrib/wrt/dhcp_lease_time $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_lease_time
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq


