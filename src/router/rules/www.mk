www-install:
	@true
	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/www
	$(MAKE) -C kromo/dd-wrt postinstall INSTALLDIR=$(INSTALLDIR)/www

www: 
	rm -rf $(INSTALLDIR)/www
ifeq ($(CONFIG_EOP_TUNNEL),y)
	$(MAKE) -C eop-tunnel www-install INSTALLDIR=$(INSTALLDIR)/www
endif
ifeq ($(CONFIG_WIREGUARD),y)
	$(MAKE) -C eop-tunnel www-install INSTALLDIR=$(INSTALLDIR)/www
endif
ifeq ($(CONFIG_WIVIZ),y)
	$(MAKE) -C wiviz2 wwwinstall INSTALLDIR=$(INSTALLDIR)/wiviz2
endif
	$(MAKE) -C kromo/dd-wrt install INSTALLDIR=$(INSTALLDIR)/www

www-distclean www-clean:
	$(MAKE) -C kromo/dd-wrt clean


