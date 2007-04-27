zebra:
ifeq ($(CONFIG_ZEBRA),y)
	$(MAKE) -C quagga CC=$(CC)
endif

zebra-install:
ifeq ($(CONFIG_ZEBRA),y)
	install -D quagga/zebra/zebra $(INSTALLDIR)/zebra/usr/sbin/zebra
	install -D quagga/ripd/ripd $(INSTALLDIR)/zebra/usr/sbin/ripd
	install -D quagga/ospfd/ospfd $(INSTALLDIR)/zebra/usr/sbin/ospfd
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/zebra
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ripd
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ospfd
else
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/zebra
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/ripd
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/ospfd
endif

