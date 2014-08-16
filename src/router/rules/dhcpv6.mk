dhcpv6:
	CC="$(CC)" \
	CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/shared -DUSE_DHCP6SRV -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/shared  -DUSE_DHCP6SRV -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/shared  -DUSE_DHCP6SRV -L$(TOP)/nvram  -L$(TOP)/libutils -lutils -lnvram -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(MAKE) -C dhcpv6 all
	
dhcpv6-install:
	install -D dhcpv6/dhcp6c $(INSTALLDIR)/dhcpv6/usr/bin/dhcp6c
	install -D dhcpv6/dhcp6s $(INSTALLDIR)/dhcpv6/usr/bin/dhcp6s

dhcpv6-clean:
	$(MAKE) -C dhcpv6 clean

dhcpv6-configure:
	cd dhcpv6 && ./configure --prefix= --with-localdbdir=/var --with-sysconfdir=/etc \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			ac_cv_func_setpgrp_void=yes \
			CC="$(CC)" \
			CXXFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/shared -DNEED_PRINTF -D_GNU_SOURCE -DUSE_DHCP6SRV -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/shared -DNEED_PRINTF -D_GNU_SOURCE  -DUSE_DHCP6SRV -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/shared -fPIC -L$(TOP)/nvram -L$(TOP)/libutils -lutils -lnvram -ffunction-sections -fdata-sections -Wl,--gc-sections"