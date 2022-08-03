antaira-agent-configure:
	@true

antaira-agent:
	make -C antaira-agent
	install -D antaira-agent/config/antaira-agent.webvpn $(TOP)/httpd/ej_temp/antaira-agent.webvpn

antaira-agent-clean:
	make -C antaira-agent clean

antaira-agent-install:
	make -C antaira-agent install
	install -D antaira-agent/antaira-quick-vpn-agent $(INSTALLDIR)/antaira-agent/usr/sbin/antaira-quick-vpn-agent
	cd $(INSTALLDIR)/openvpn/usr/sbin && ln -sf openvpn antaira-agent-vpn
	install -D antaira-agent/config/antaira-agent.nvramconfig $(INSTALLDIR)/antaira-agent/etc/config/antaira-agent.nvramconfig
	install -D antaira-agent/config/antaira-agent.webvpn $(INSTALLDIR)/antaira-agent/etc/config/antaira-agent.webvpn

