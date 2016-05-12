igmp-proxy-configure:
	cd igmp-proxy && ./configure --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99"

igmp-proxy-clean:
	make -C igmp-proxy clean

igmp-proxy:
ifeq ($(CONFIG_DIST),"micro")
	make -C igmp-proxy
else
ifeq ($(CONFIG_DIST),"micro-special")
	make -C igmp-proxy
else
	make -C igmp-proxy CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99"
endif
endif

igmp-proxy-install:
	install -D igmp-proxy/src/igmpproxy $(INSTALLDIR)/igmp-proxy/usr/sbin/igmprt

