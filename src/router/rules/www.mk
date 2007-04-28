www www-%: 
ifeq ($(CONFIG_EOP_TUNNEL),y)
	$(MAKE) -C eop-tunnel install
endif
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) $* INSTALLDIR=$(INSTALLDIR)/www
else
	$(MAKE) -C www/$(WEB_PAGE) $* INSTALLDIR=$(INSTALLDIR)/www
endif

www-distclean www-clean:
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) clean
else
	$(MAKE) -C www/$(WEB_PAGE) clean
endif


