wireguard: libmnl
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR)
	cd wireguard/src ; $(MAKE) LIBMNL_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" LIBMNL_LDLIBS="$(LDLTO) -L$(TOP)/libmnl/src/.libs -lmnl" RUNSTATEDIR=/var/run tools 

wireguard-install:
	install -D wireguard/src/wireguard.ko $(INSTALLDIR)/wireguard/lib/modules/$(KERNELRELEASE)/wireguard.ko
	install -D wireguard/src/tools/wg $(INSTALLDIR)/wireguard/usr/bin/wg

wireguard-configure:
	@true

wireguard-clean:
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR) clean
	cd wireguard/src/tools ; $(MAKE) M="$(TOP)/wireguard/src" clean

