igmp-proxy-configure:
	cd igmp-proxy && ./configure --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE  -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99" LDFLAGS="-L$(TOP)/libutils -lutils"

igmp-proxy-clean:
	make -C igmp-proxy clean

igmp-proxy:
	make -C igmp-proxy

igmp-proxy-install:
	install -D igmp-proxy/src/igmpproxy $(INSTALLDIR)/igmp-proxy/usr/sbin/igmprt

