ifneq ($(CONFIG_IPV6),y)
export DNSMASQ_MAKEFLAGS:=-DNO_IPV6
endif


DNSMASQ_COPTS += $(MIPS16_OPT) -DNO_AUTH $(LTO)
ifeq ($(ARCH),armeb)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),arm)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),mips64)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),i386)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),x86_64)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),aarch64)
DNSMASQ_COPTS += -DNEED_PRINTF
endif

ifeq ($(CONFIG_DNSSEC),y)
export DNSSEC_MAKEFLAGS:=-DHAVE_DNSSEC -DNO_NETTLE_ECC -I$(TOP) -I$(TOP)/gmp
#export DNSSEC_LINKFLAGS:=-L$(TOP)/pcre/.libs -lpcre -L$(TOP)/zlib -lz -L$(TOP)/nettle/.lib -lnettle -lhogweed -L$(TOP)/gmp/.libs -lgmp
export DNSSEC_LINKFLAGS:=-L$(TOP)/pcre/.libs -lpcre -L$(TOP)/zlib -lz $(TOP)/nettle/libhogweed.a $(TOP)/nettle/libnettle.a $(TOP)/gmp/.libs/libgmp.a
endif

dnsmasq-clean:
	$(MAKE) -j 4 -C dnsmasq CFLAGS="$(COPTS)" clean
	$(MAKE) -j 4 -C dnsmasq/contrib/lease-tools CFLAGS="$(COPTS)" clean



dnsmasq:
	$(MAKE) -C dnsmasq clean
ifeq ($(CONFIG_DNSMASQ_TFTP),y)
	$(MAKE) -j 4 -C dnsmasq CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
ifeq ($(CONFIG_DIST),"micro")
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
ifeq ($(CONFIG_DIST),"micro-special")
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
	$(MAKE) -j 4 -C dnsmasq "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) $(DNSSEC_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_LINKFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
endif
endif
endif
	$(MAKE) -j 4 -C dnsmasq/contrib/lease-tools CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO)"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/contrib/lease-tools/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D dnsmasq/contrib/lease-tools/dhcp_lease_time $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_lease_time
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq
#ifeq ($(CONFIG_BUFFALO),y)
#	install -D udhcpd/config/dhcpd.webservices.buffalo $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.webservices
#else
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.webservices
#endif
	install -D udhcpd/config/dhcpd.startup $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.startup
	install -D dnsmasq/configs/etc/rfc6761.conf $(INSTALLDIR)/dnsmasq/etc/rfc6761.conf
ifeq ($(CONFIG_DNSSEC),y)
	install -D dnsmasq/trust-anchors.conf $(INSTALLDIR)/dnsmasq/etc/trust-anchors.conf
endif


