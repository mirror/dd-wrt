DNSMASQ_PATH=dnsmasq

ifneq ($(CONFIG_IPV6),y)
export DNSMASQ_MAKEFLAGS:=-DNO_IPV6
endif

ifeq ($(CONFIG_IPSET),y)
export DNSMASQ_MAKEFLAGS += -DHAVE_IPSET
endif

DNSMASQ_COPTS += $(MIPS16_OPT) -DNO_AUTH $(LTO) -std=gnu99
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
ifneq ($(CONFIG_NOMESSAGE),y)
DNSMASQ_COPTS += -DNEED_PRINTF
endif
INOTIFY_SUPPORT = $(shell grep CONFIG_INOTIFY_USER=y $(LINUXDIR)/.config)
ifneq ($(INOTIFY_SUPPORT),CONFIG_INOTIFY_USER=y)
DNSMASQ_COPTS += -DNO_INOTIFY
endif
ifeq ($(ARCHITECTURE),broadcom)
ifneq ($(CONFIG_80211AC),y)
DNSMASQ_COPTS += -DNO_INOTIFY
endif
endif
DNSSEC_LINKFLAGS = -L$(TOP)/libutils -lshutils -L$(TOP)/nvram -lnvram
ifeq ($(CONFIG_DNSSEC),y)
export DNSSEC_MAKEFLAGS += -DHAVE_DNSSEC -DNO_NETTLE_ECC -DHAVE_NETTLEHASH -I$(TOP) -I$(TOP)/gmp
#export DNSSEC_LINKFLAGS:=-L$(TOP)/pcre/.libs -lpcre -L$(TOP)/zlib -lz -L$(TOP)/nettle/.lib -lnettle -lhogweed -L$(TOP)/gmp/.libs -lgmp
export DNSSEC_LINKFLAGS += -L$(TOP)/pcre/.libs -lpcre -L$(TOP)/zlib -lz $(TOP)/nettle/libhogweed.a $(TOP)/nettle/libnettle.a $(TOP)/gmp/.libs/libgmp.a
endif

dnsmasq-clean:
	$(MAKE) -j 4 -C $(DNSMASQ_PATH) CFLAGS="$(COPTS)" clean
	$(MAKE) -j 4 -C $(DNSMASQ_PATH)/contrib/lease-tools CFLAGS="$(COPTS)" clean



dnsmasq: nettle gmp
	install -D udhcpd/config/dhcpd.webservices httpd/ej_temp/dhcpd.webservices
	$(MAKE) -C $(DNSMASQ_PATH) clean
ifeq ($(CONFIG_DNSMASQ_TFTP),y)
	$(MAKE) -j 4 -C $(DNSMASQ_PATH) CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_MAKEFLAGS)  -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_LINKFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
ifeq ($(CONFIG_DIST),"micro")
	$(MAKE) -j 4 -C $(DNSMASQ_PATH) "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_LINKFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
ifeq ($(CONFIG_DIST),"micro-special")
	$(MAKE) -j 4 -C $(DNSMASQ_PATH) "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_LINKFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
else
	$(MAKE) -j 4 -C $(DNSMASQ_PATH) "COPTS=-DNO_TFTP" CFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSMASQ_MAKEFLAGS) $(DNSSEC_MAKEFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) $(DNSSEC_LINKFLAGS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)"
endif
endif
endif
	$(MAKE) -j 4 -C $(DNSMASQ_PATH)/contrib/lease-tools CFLAGS="$(COPTS) $(DNSMASQ_COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(DNSMASQ_COPTS) -DNO_LOG -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO)"

dnsmasq-install:
	install -D $(DNSMASQ_PATH)/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D $(DNSMASQ_PATH)/contrib/lease-tools/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D $(DNSMASQ_PATH)/contrib/lease-tools/dhcp_lease_time $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_lease_time
	install -D $(DNSMASQ_PATH)/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/dnsmasq/etc/config/dhcpd.webservices
	install -D $(DNSMASQ_PATH)/configs/etc/rfc6761.conf $(INSTALLDIR)/dnsmasq/etc/rfc6761.conf
ifeq ($(CONFIG_DNSSEC),y)
	install -D $(DNSMASQ_PATH)/trust-anchors.conf $(INSTALLDIR)/dnsmasq/etc/trust-anchors.conf
endif


