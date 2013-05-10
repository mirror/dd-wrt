ifneq ($(CONFIG_IPV6),y)
export DNSMASQ_MAKEFLAGS:=-DNO_IPV6
endif
ifeq ($(PLATFORM),mipsel-uclibc)
DNSMASQ_COPTS += -minterlink-mips16 -mips16
endif
ifeq ($(PLATFORM),mips-uclibc)
DNSMASQ_COPTS += -minterlink-mips16 -mips16
endif

dnsmasq-clean:
	$(MAKE) -j 4 -C dnsmasq CFLAGS="$(COPTS)" clean
	$(MAKE) -j 4 -C dnsmasq/contrib/wrt CFLAGS="$(COPTS)" clean



dnsmasq:
	$(MAKE) -C dnsmasq clean
ifeq ($(CONFIG_DNSMASQ_TFTP),y)
	$(MAKE) -j 4 -C dnsmasq COPTS=-DHAVE_BROKEN_RTC CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections"
else
ifeq ($(CONFIG_DIST),"micro")
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DHAVE_BROKEN_RTC -DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG $(DNSMASQ_MAKEFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections"
else
ifeq ($(CONFIG_DIST),"micro-special")
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DHAVE_BROKEN_RTC -DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG $(DNSMASQ_MAKEFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections"
else
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DHAVE_BROKEN_RTC -DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections" 
endif
endif
endif
	$(MAKE) -j 4 -C dnsmasq/contrib/wrt CFLAGS="$(COPTS)  $(DNSMASQ_COPTS)"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/contrib/wrt/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D dnsmasq/contrib/wrt/dhcp_lease_time $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_lease_time
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq
#ifeq ($(CONFIG_BUFFALO),y)
#	install -D udhcpd/config/dhcpd.webservices.buffalo $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.webservices
#else
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.webservices
#endif
	install -D udhcpd/config/dhcpd.startup $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.startup


