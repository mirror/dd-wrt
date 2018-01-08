WG_DIR=WireGuard-0.0.20171221
export KERNELDIR:=$(LINUX_DIR)

wireguard: 
	cd $(WG_DIR)/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/$(WG_DIR)/src"
	cd $(WG_DIR)/src ; $(MAKE) LIBMNL_CFLAGS="-I$(TOP)/libmnl/include" LIBMNL_LDLIBS="-L$(TOP)/libmnl/src/.libs -lmnl" tools
#	$(MAKE) M="$(WG_DIR)/src" tools

wireguard-install:
	@true
	install -D libmnl/src/.libs/libmnl.so.0.2.0 $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.2.0
	cd $(INSTALLDIR)/libmnl/usr/lib ; ln -s libmnl.so.0.2.0 libmnl.so  ; true
	cd $(INSTALLDIR)/libmnl/usr/lib ; ln -s libmnl.so.0.2.0 libmnl.so.0  ; true

wireguard-configure:
	#cd libmnl && ./configure --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-gcc 
	true
	#cd libmnl && ./configure --host=$(ARCH)-linux

wireguard-clean:
	cd $(WG_DIR)/src ; $(MAKE) -C $(LINUXDIR) M="$(TOP)/$(WG_DIR)/src" clean
	cd $(WG_DIR)/src/tools ; $(MAKE) M="$(TOP)/$(WG_DIR)/src" clean

