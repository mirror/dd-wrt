igmp-proxy-configure:
	cd igmp-proxy && ./autogen.sh
	cd igmp-proxy && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99"

igmp-proxy-clean:
	make -C igmp-proxy clean

igmp-proxy:
ifeq ($(CONFIG_DIST),"micro")
	make -C igmp-proxy
else
ifeq ($(CONFIG_DIST),"micro-special")
	make -C igmp-proxy
else
	make -C igmp-proxy CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -std=gnu99"
endif
endif

igmp-proxy-install:
	install -D igmp-proxy/src/igmpproxy $(INSTALLDIR)/igmp-proxy/usr/sbin/igmprt

