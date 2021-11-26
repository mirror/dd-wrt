wireguard: libmnl
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR)
	cd wireguard/src ; $(MAKE) LIBMNL_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" LIBMNL_LDLIBS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/libmnl/src/.libs -lmnl" RUNSTATEDIR=/var/run tools 

wireguard-install:
	install -D wireguard/src/wireguard.ko $(INSTALLDIR)/wireguard/lib/modules/$(KERNELRELEASE)/wireguard.ko
	install -D wireguard/src/tools/wg $(INSTALLDIR)/wireguard/usr/bin/wg
	#install -d wireguard/$(INSTALLDIR)/usr/bin
	install -c wireguard/$(MODULE)*.sh $(INSTALLDIR)/wireguard/usr/bin
	#install -c will set x 
	ln -s $(INSTALLDIR)/wireguard/usr/bin/is-mounted.sh $(INSTALLDIR)/wireguard/usr/bin/is-mounted
	#cp wireguard/$(MODULE)*.sh $(INSTALLDIR)/wireguard/usr/bin
	#chmod 700 $(INSTALLDIR)/wireguard/usr/bin/$(MODULE)*.sh


wireguard-configure:
	@true

wireguard-clean:
	cd wireguard/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/wireguard/src" KERNELDIR=$(LINUX_DIR) clean
	cd wireguard/src/tools ; $(MAKE) M="$(TOP)/wireguard/src" clean

