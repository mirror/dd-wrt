mmc:
	$(MAKE) -C mmc_sdhc

mmc-install:
ifeq ($(CONFIG_BCM5354),y)
	install -D  mmc_sdhc/mmc.o $(INSTALLDIR)/mmc/lib/modules/2.4.37/mmc.o
else
	install -D  mmc_sdhc/mmc.o $(INSTALLDIR)/mmc/lib/modules/2.4.35/mmc.o
endif
	install -D  mmc_sdhc/config/mmc.webconfig $(INSTALLDIR)/mmc/etc/config/mmc.webconfig
	install -D  mmc_sdhc/config/mmc.startup $(INSTALLDIR)/mmc/etc/config/mmc.startup
	install -D  mmc_sdhc/config/mmc.nvramconfig $(INSTALLDIR)/mmc/etc/config/mmc.nvramconfig

mmc-clean:
	$(MAKE) -C mmc_sdhc clean
