pptp-client:
	$(MAKE) -C pptp-client

pptp-client-clean:
	$(MAKE) -C pptp-client clean

pptp-client-install:
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifneq ($(CONFIG_NEWMEDIA),y)
	install -D pptp-client/config/pptpd_client.ip-up $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-up
	install -D pptp-client/config/pptpd_client.ip-down $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-down
	install -D pptp-client/config/pptpd_client.nvramconfig $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.nvramconfig
	install -D pptp-client/config/pptpd_client.options $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.options
	install -D pptp-client/config/pptpd_client.sh $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.sh
	install -D pptp-client/config/pptpd_client.startup $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.startup
	install -D pptp-client/config/pptpd_client.vpn $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.vpn
else
	install -D pptp-client/config2/pptpd_client.ip-up $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-up
	install -D pptp-client/config2/pptpd_client.ip-down $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-down
	install -D pptp-client/config2/pptpd_client.nvramconfig $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.nvramconfig
	install -D pptp-client/config2/pptpd_client.options $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.options
	install -D pptp-client/config2/pptpd_client.sh $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.sh
	install -D pptp-client/config2/pptpd_client.wanup $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.wanup
	install -D pptp-client/config2/pptpd_client.vpn $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.vpn
endif
endif
endif
endif