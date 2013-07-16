wireless-tools-clean:
	make -C wireless-tools clean

wireless-tools:
	make -C wireless-tools CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT)" all iwmulticall

wireless-tools-install:
ifneq ($(CONFIG_JUSTLIBIW),y)
	make -C wireless-tools INSTALL_DIR=$(INSTALLDIR)/wireless-tools/usr/sbin install-iwmulticall
endif
	make -C wireless-tools INSTALL_LIB=$(INSTALLDIR)/wireless-tools/lib install-dynamic

