mmc-fonera:
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera modules

mmc-fonera-install:
	install -D  mmc-fonera/mmc.ko $(INSTALLDIR)/mmc-fonera/lib/modules/$(KERNELRELEASE)/mmc.ko
	install -D  mmc/config/mmc.webconfig $(INSTALLDIR)/mmc/etc/config/mmc.webconfig
	install -D  mmc/config/mmc.startup $(INSTALLDIR)/mmc/etc/config/mmc.startup
	install -D  mmc/config/mmc.nvramconfig $(INSTALLDIR)/mmc/etc/config/mmc.nvramconfig

mmc-fonera-clean:
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera clean	
