igmp-proxy-configure:
	cd igmp-proxy && ./configure --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99"

igmp-proxy-clean:
	make -C igmp-proxy clean

igmp-proxy:
	make -C igmp-proxy

igmp-proxy-install:
	install -D igmp-proxy/src/igmp-proxy $(INSTALLDIR)/igmp-proxy/usr/sbin

