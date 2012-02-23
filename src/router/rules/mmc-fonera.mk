mmc-fonera:
ifneq ($(CONFIG_HOTPLUG2),y)
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera modules
endif
	@true	

mmc-fonera-install:
ifneq ($(CONFIG_HOTPLUG2),y)
	install -D  mmc-fonera/mmc.ko $(INSTALLDIR)/mmc-fonera/lib/modules/$(KERNELRELEASE)/mmc.ko
	install -D  mmc-fonera/config/mmc.webconfig $(INSTALLDIR)/mmc-fonera/etc/config/mmc.webconfig
	install -D  mmc-fonera/config/mmc.startup $(INSTALLDIR)/mmc-fonera/etc/config/mmc.startup
	install -D  mmc-fonera/config/mmc.nvramconfig $(INSTALLDIR)/mmc-fonera/etc/config/mmc.nvramconfig
endif
	@true	

mmc-fonera-clean:
ifneq ($(CONFIG_HOTPLUG2),y)
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/mmc-fonera clean	
endif
	@true	
