WG_DIR=WireGuard-0.0.20171221
export KERNELDIR:=$(LINUX_DIR)

wireguard: 
	cd $(WG_DIR)/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/$(WG_DIR)/src"
	cd $(WG_DIR)/src ; $(MAKE) LIBMNL_CFLAGS="-I$(TOP)/libmnl/include" LIBMNL_LDLIBS="-L$(TOP)/libmnl/src/.libs -lmnl" tools

wireguard-install:
	install -D $(WG_DIR)/src/wireguard.ko $(INSTALLDIR)/wireguard/lib/modules/$(KERNELRELEASE)/wireguard.ko
	install -D $(WG_DIR)/src/tools/wg $(INSTALLDIR)/wireguard/usr/bin/wg

wireguard-configure:
	@true

wireguard-clean:
	cd $(WG_DIR)/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/$(WG_DIR)/src" clean
	cd $(WG_DIR)/src/tools ; $(MAKE) M="$(TOP)/$(WG_DIR)/src" clean

