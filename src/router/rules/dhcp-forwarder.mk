dhcpforwarder-configure:
	cd dhcpforwarder && ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) --sysconfdir=/tmp/dhcp-fwd CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DHAVE_MALLOC=1 -Drpl_malloc=malloc -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG" LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG"

dhcpforwarder:
	$(MAKE) -C dhcpforwarder

dhcpforwarder-install:
	install -D dhcpforwarder/dhcp-fwd $(INSTALLDIR)/dhcpforwarder/usr/sbin/dhcpfwd
	
dhcpforwarder-clean:
	if test -e "dhcpforwarder/Makefile"; then $(MAKE) -C dhcpforwarder clean; fi
	
