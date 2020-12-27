ppp:
ifeq ($(CONFIG_PPPOERELAY),y)
	install -D rp-pppoe/config/pppoe-relay.webservices httpd/ej_temp/pppoe-relay.webservices
endif
	$(MAKE) -C rp-pppoe/src

ppp-clean:
	$(MAKE) -C rp-pppoe/src clean

ppp-install:
#	install -D rp-pppoe/src/pppoe $(INSTALLDIR)/ppp/usr/sbin/pppoe
#	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe
ifeq ($(CONFIG_PPPOERELAY),y)
	install -D rp-pppoe/src/pppoe-multi $(INSTALLDIR)/ppp/usr/sbin/pppoe-multi
	cd $(INSTALLDIR)/ppp/usr/sbin && ln -sf pppoe-multi pppoe-relay
	install -D rp-pppoe/config/pppoe-relay.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-relay.nvramconfig
	install -D rp-pppoe/config/pppoe-relay.webservices $(INSTALLDIR)/ppp/etc/config/pppoe-relay.webservices
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
endif
ifeq ($(CONFIG_PPPOESERVER),y)
	install -D rp-pppoe/src/pppoe-multi $(INSTALLDIR)/ppp/usr/sbin/pppoe-multi
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-multi
	cd $(INSTALLDIR)/ppp/usr/sbin && ln -sf pppoe-multi pppoe-server
	install -D rp-pppoe/config/pppoe-server.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-server.nvramconfig
endif
ifeq ($(CONFIG_PPPOESNIFF),y)
	install -D rp-pppoe/src/pppoe-sniff $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
endif
ifeq ($(CONFIG_PPPOEDISCOVERY),y)
	install -D rp-pppoe/src/pppoe $(INSTALLDIR)/ppp/usr/sbin/pppoe
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe
endif


