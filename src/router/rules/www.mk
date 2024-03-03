www-install:
	@true
	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/www
	make -C kromo/dd-wrt postinstall INSTALLDIR=$(INSTALLDIR)/www

www: 
	rm -rf $(INSTALLDIR)/www
ifeq ($(CONFIG_EOP_TUNNEL),y)
	make -C eop-tunnel www-install INSTALLDIR=$(INSTALLDIR)/www
endif
ifeq ($(CONFIG_WIREGUARD),y)
	make -C eop-tunnel www-install INSTALLDIR=$(INSTALLDIR)/www
endif
ifeq ($(CONFIG_WIVIZ),y)
	make -C wiviz2 wwwinstall INSTALLDIR=$(INSTALLDIR)/wiviz2
endif
	make -C kromo/dd-wrt install INSTALLDIR=$(INSTALLDIR)/www

www-distclean www-clean:
	make -C kromo/dd-wrt clean


