mmc-fonera:
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera modules

mmc-fonera-install:
	install -D  mmc-fonera/mmc.ko $(INSTALLDIR)/mmc-fonera/lib/modules/$(KERNELRELEASE)/mmc.ko
	install -D  mmc-fonera/config/mmc.webconfig $(INSTALLDIR)/mmc-fonera/etc/config/mmc.webconfig
	install -D  mmc-fonera/config/mmc.startup $(INSTALLDIR)/mmc-fonera/etc/config/mmc.startup
	install -D  mmc-fonera/config/mmc.nvramconfig $(INSTALLDIR)/mmc-fonera/etc/config/mmc.nvramconfig

mmc-fonera-clean:
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera clean	
