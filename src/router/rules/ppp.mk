ppp:
	$(MAKE) -C rp-pppoe-3.5/src

ppp-clean:
	$(MAKE) -C rp-pppoe-3.5/src clean

ppp-install:
#	install -D rp-pppoe-3.5/src/pppoe $(INSTALLDIR)/ppp/usr/sbin/pppoe
#	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe
ifeq ($(CONFIG_PPPOERELAY),y)
	install -D rp-pppoe-3.5/src/pppoe-multi $(INSTALLDIR)/ppp/usr/sbin/pppoe-multi
	cd $(INSTALLDIR)/ppp/usr/sbin && ln -sf pppoe-multi pppoe-relay
	install -D rp-pppoe-3.5/config/pppoe-relay.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-relay.nvramconfig
	install -D rp-pppoe-3.5/config/pppoe-relay.webservices $(INSTALLDIR)/ppp/etc/config/pppoe-relay.webservices
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
endif
ifeq ($(CONFIG_PPPOESERVER),y)
	install -D rp-pppoe-3.5/src/pppoe-multi $(INSTALLDIR)/ppp/usr/sbin/pppoe-multi
	cd $(INSTALLDIR)/ppp/usr/sbin && ln -sf pppoe-multi pppoe-server
	install -D rp-pppoe-3.5/config/pppoe-server.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-server.nvramconfig
endif
ifeq ($(CONFIG_PPPOESNIFF),y)
	install -D rp-pppoe-3.5/src/pppoe-sniff $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
endif

