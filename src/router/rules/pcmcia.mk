pcmcia-clean:
	$(MAKE) -C pcmcia-cs-3.2.8 CC=$(CC) LINUXDIR=$(LINUXDIR) clean

pcmcia:
#	cd pcmcia-cs-3.2.8 && ./Configure
	$(MAKE) -C pcmcia-cs-3.2.8 CC=$(CC) LINUXDIR=$(LINUXDIR) all 

pcmcia-install:
	install -D pcmcia-cs-3.2.8/cardmgr/cardmgr $(INSTALLDIR)/pcmcia/usr/sbin/cardmgr
	install -D pcmcia-cs-3.2.8/cardmgr/cardctl $(INSTALLDIR)/pcmcia/usr/sbin/cardctl
	install -D pcmcia-cs-3.2.8/etc/sysconfig/pcmcia $(INSTALLDIR)/pcmcia/etc/sysconfig/pcmcia
	install -D pcmcia-cs-3.2.8/etc/config $(INSTALLDIR)/pcmcia/etc/pcmcia/config
	$(STRIP) $(INSTALLDIR)/pcmcia/usr/sbin/cardmgr
	$(STRIP) $(INSTALLDIR)/pcmcia/usr/sbin/cardctl
	install pcmcia-cs-3.2.8/etc/*.* $(INSTALLDIR)/pcmcia/etc/pcmcia
	install pcmcia-cs-3.2.8/etc/serial $(INSTALLDIR)/pcmcia/etc/pcmcia/serial 
	install pcmcia-cs-3.2.8/etc/shared $(INSTALLDIR)/pcmcia/etc/pcmcia/shared 
	install pcmcia-cs-3.2.8/etc/gprs_net $(INSTALLDIR)/pcmcia/etc/gprs_net
	install pcmcia-cs-3.2.8/etc/huawei_gprs_net  $(INSTALLDIR)/pcmcia/etc/huawei_gprs_net
	install pcmcia-cs-3.2.8/etc/pin_code $(INSTALLDIR)/pcmcia/etc/pin_code
	install pcmcia-cs-3.2.8/etc/card_info $(INSTALLDIR)/pcmcia/etc/card_info
	install pcmcia-cs-3.2.8/etc/card_hwver $(INSTALLDIR)/pcmcia/etc/card_hwver
	install pcmcia-cs-3.2.8/etc/huawei_card_hwver $(INSTALLDIR)/pcmcia/etc/huawei_card_hwver
	install pcmcia-cs-3.2.8/etc/search_net_opt $(INSTALLDIR)/pcmcia/etc/search_net_opt
	install pcmcia-cs-3.2.8/etc/search_net_nov $(INSTALLDIR)/pcmcia/etc/search_net_nov
	install pcmcia-cs-3.2.8/etc/cimi $(INSTALLDIR)/pcmcia/etc/cimi
	install pcmcia-cs-3.2.8/etc/csca_u630 $(INSTALLDIR)/pcmcia/etc/csca_u630
	install pcmcia-cs-3.2.8/etc/cis/SW_8xx_SER.dat $(INSTALLDIR)/pcmcia/etc/pcmcia/SW_8xx_SER.dat

