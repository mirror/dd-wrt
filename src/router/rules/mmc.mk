mmc:
	$(MAKE) -C mmc

mmc-install:
	install -D  mmc/config/mmc.webconfig $(INSTALLDIR)/mmc/etc/config/mmc.webconfig
	install -D  mmc/config/mmc.startup $(INSTALLDIR)/mmc/etc/config/mmc.startup
	install -D  mmc/config/mmc.nvramconfig $(INSTALLDIR)/mmc/etc/config/mmc.nvramconfig

mmc-clean:
	$(MAKE) -C mmc clean
