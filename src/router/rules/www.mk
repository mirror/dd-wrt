www-install:
	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/www
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) install INSTALLDIR=$(INSTALLDIR)/www
else
	$(MAKE) -C www/$(WEB_PAGE) install INSTALLDIR=$(INSTALLDIR)/www
endif

www: 
	rm -rf $(INSTALLDIR)/www
ifeq ($(CONFIG_EOP_TUNNEL),y)
	$(MAKE) -C eop-tunnel www-install INSTALLDIR=$(INSTALLDIR)/www
endif
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) install INSTALLDIR=$(INSTALLDIR)/www
else
	$(MAKE) -C www/$(WEB_PAGE) install INSTALLDIR=$(INSTALLDIR)/www
endif

www-distclean www-clean:
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) clean
else
	$(MAKE) -C www/$(WEB_PAGE) clean
endif


