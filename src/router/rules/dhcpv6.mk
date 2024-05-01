
dhcpv6:
	CC="$(CC)" \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -DNEED_PRINTF   -D_GNU_SOURCE -I$(TOP)/shared -DUSE_DHCP6SRV -DNOCONFIG_DEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -DNEED_PRINTF  -D_GNU_SOURCE -I$(TOP)/shared  -DUSE_DHCP6SRV -DNOCONFIG_DEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -DNEED_PRINTF  -D_GNU_SOURCE -I$(TOP)/shared  -DUSE_DHCP6SRV -DNOCONFIG_DEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(JFLAGS) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/libutils/ -lutils -lshutils -L$(TOP)/nvram -lnvram" \
	$(MAKE) -C dhcpv6 all
	
dhcpv6-install:
	install -D dhcpv6/multicall $(INSTALLDIR)/dhcpv6/usr/bin/dhcp6_multicall
	cd $(INSTALLDIR)/dhcpv6/usr/bin && ln -sf dhcp6_multicall dhcp6c
	cd $(INSTALLDIR)/dhcpv6/usr/bin && ln -sf dhcp6_multicall dhcp6s

dhcpv6-clean:
	$(MAKE) -C dhcpv6 clean

dhcpv6-configure: nvram
	cd dhcpv6 && ./configure --prefix= --with-localdbdir=/var --with-sysconfdir=/etc \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			ac_cv_func_setpgrp_void=yes \
			CC="$(CC)" \
			CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -I$(TOP)/shared -DNEED_PRINTF  -D_GNU_SOURCE -DUSE_DHCP6SRV -DNOCONFIG_DEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -I$(TOP)/shared -DNEED_PRINTF  -D_GNU_SOURCE  -DUSE_DHCP6SRV -DNOCONFIG_DEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(JFLAGS) -I$(TOP)/shared -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libutils/ -lutils -lshutils -L$(TOP)/nvram -lnvram"
