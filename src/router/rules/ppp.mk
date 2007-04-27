ppp:
	$(MAKE) -C rp-pppoe-3.5/src

ppp-clean:
	$(MAKE) -C rp-pppoe-3.5/src clean

ppp-install:
#	install -D rp-pppoe-3.5/src/pppoe $(INSTALLDIR)/ppp/usr/sbin/pppoe
#	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe
ifeq ($(CONFIG_PPPOERELAY),y)
	install -D rp-pppoe-3.5/src/pppoe-relay $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
	install -D rp-pppoe-3.5/config/pppoe-relay.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-relay.nvramconfig
	install -D rp-pppoe-3.5/config/pppoe-relay.webservices $(INSTALLDIR)/ppp/etc/config/pppoe-relay.webservices
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
endif
ifeq ($(CONFIG_PPPOESERVER),y)
	install -D rp-pppoe-3.5/config/pppoe-server.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-server.nvramconfig
#	install -D rp-pppoe-3.5/config/pppoe-server.webservices $(INSTALLDIR)/ppp/etc/config/pppoe-server.webservices
	install -D rp-pppoe-3.5/src/pppoe-server $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
endif
ifeq ($(CONFIG_PPPOESNIFF),y)
	install -D rp-pppoe-3.5/src/pppoe-sniff $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
endif

