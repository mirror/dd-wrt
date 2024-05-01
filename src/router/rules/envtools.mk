
envtools:
	make -C uboot defconfig	
	make -C uboot envtools ARCH="sandbox" TARGET_CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -DNEED_PRINTF" 

envtools-install:
	install -d $(INSTALLDIR)/envtools/usr/sbin
	install -d $(INSTALLDIR)/envtools/etc
	install uboot/tools/env/fw_printenv $(INSTALLDIR)/envtools/usr/sbin
	cd $(INSTALLDIR)/envtools/usr/sbin && ln -sf fw_printenv fw_setenv
	cd $(INSTALLDIR)/envtools/etc && ln -sf /tmp/fw_env.config fw_env.config

envtools-clean:
	make -C uboot clean



