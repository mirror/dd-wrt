dhcpforwarder-configure:
	cd dhcpforwarder && ./configure --host=$(ARCH)-linux-elf --sysconfdir=/tmp/dhcp-fwd CC=$(ARCH)-linux-uclibc-gcc CFLAGS="$(COPTS) -DHAVE_MALLOC=1 -Drpl_malloc=malloc -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" 

dhcpforwarder:
	$(MAKE) -C dhcpforwarder

dhcpforwarder-install:
	install -D dhcpforwarder/dhcpfwd $(INSTALLDIR)/dhcpforwarder/usr/sbin/dhcpfwd
	
dhcpforwarder-clean:
	if test -e "dhcpforwarder/Makefile"; then $(MAKE) -C dhcpforwarder clean; fi
	