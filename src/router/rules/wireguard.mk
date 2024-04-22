wireguard: libmnl
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR)
endif
endif
	cd wireguard/src ; $(MAKE) LIBMNL_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" LIBMNL_LDLIBS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/libmnl/src/.libs -lmnl" RUNSTATEDIR=/var/run tools 
	cd wireguard/obfuscation/kernel ; $(MAKE)

wireguard-install:
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	install -D wireguard/src/wireguard.ko $(INSTALLDIR)/wireguard/lib/modules/$(KERNELRELEASE)/wireguard.ko
endif
endif
	install -D wireguard/src/tools/wg $(INSTALLDIR)/wireguard/usr/bin/wg
	#install -d wireguard/$(INSTALLDIR)/usr/bin
	install -c wireguard/$(MODULE)*.sh $(INSTALLDIR)/wireguard/usr/bin
	#install -c will set x 
	cd $(INSTALLDIR)/wireguard/usr/bin && ln -f -s is-mounted.sh is-mounted
	#cp wireguard/$(MODULE)*.sh $(INSTALLDIR)/wireguard/usr/bin
	#chmod 700 $(INSTALLDIR)/wireguard/usr/bin/$(MODULE)*.sh
	cd wireguard/obfuscation/kernel ; $(MAKE) install


wireguard-configure:
	@true

wireguard-clean:
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR) clean
endif
endif
	cd wireguard/src/tools ; $(MAKE) M="$(TOP)/wireguard/src" clean
	cd wireguard/obfuscation/kernel ; $(MAKE) clean

