dhcpforwarder-configure:
	cd dhcpforwarder && rm -f config.cache
	cd dhcpforwarder && libtoolize
	cd dhcpforwarder && aclocal
	cd dhcpforwarder && autoconf
	cd dhcpforwarder && autoheader
	cd dhcpforwarder && autoreconf -vfi
	cd dhcpforwarder && ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) --sysconfdir=/tmp/dhcp-fwd CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DHAVE_MALLOC=1 -Drpl_malloc=malloc -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG" LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG"

dhcpforwarder:
	$(MAKE) -C dhcpforwarder

dhcpforwarder-install:
	install -D dhcpforwarder/dhcp-fwd $(INSTALLDIR)/dhcpforwarder/usr/sbin/dhcpfwd
	
dhcpforwarder-clean:
	if test -e "dhcpforwarder/Makefile"; then $(MAKE) -C dhcpforwarder clean; fi
	
