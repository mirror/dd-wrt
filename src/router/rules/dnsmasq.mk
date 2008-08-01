dnsmasq:
ifeq ($(CONFIG_DNSMASQ_TFTP),y)
	$(MAKE) -C dnsmasq COPTS=-DHAVE_BROKEN_RTC CFLAGS="$(COPTS)"
else
ifeq ($(CONFIG_WRK54G),y)
	$(MAKE) -C dnsmasq "COPTS=-DHAVE_BROKEN_RTC -DNO_TFTP" CFLAGS="$(COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections"
else
	$(MAKE) -C dnsmasq "COPTS=-DHAVE_BROKEN_RTC -DNO_TFTP" CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" 
endif
endif
	$(MAKE) -C dnsmasq/contrib/wrt CFLAGS="$(COPTS)"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/contrib/wrt/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D dnsmasq/contrib/wrt/dhcp_lease_time $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_lease_time
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq


