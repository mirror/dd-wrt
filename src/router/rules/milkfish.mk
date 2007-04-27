milkfish:
	@true

milkfish-install:
	install -d -m0755 $(INSTALLDIR)/milkfish/etc/openser
	install -m0644 milkfish/etc/openser/milkfish_openser.cfg $(INSTALLDIR)/milkfish/etc/openser/milkfish_openser.cfg
	install -d -m0755 $(INSTALLDIR)/milkfish/etc/openser/dbtext
	install -m0644 milkfish/etc/openser/dbtext/aliases $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/aliases.empty $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/grp $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/grp.empty $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/location $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/location.empty $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/subscriber $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/subscriber.empty $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/uri $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/uri.empty $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -m0644 milkfish/etc/openser/dbtext/version $(INSTALLDIR)/milkfish/etc/openser/dbtext/
	install -d -m0755 $(INSTALLDIR)/milkfish/etc/config
	install -m0755 milkfish/etc/config/milkfish.startup $(INSTALLDIR)/milkfish/etc/config/
	install -m0755 milkfish/etc/config/milkfish.netup $(INSTALLDIR)/milkfish/etc/config/
	install -d -m0755 $(INSTALLDIR)/milkfish/usr/sbin
	install -m0755 milkfish/usr/sbin/milkfish_services $(INSTALLDIR)/milkfish/usr/sbin/
	install -m0755 milkfish/usr/sbin/mf_sip_tracer.sh $(INSTALLDIR)/milkfish/usr/sbin
	install -d -m0755 $(INSTALLDIR)/milkfish/usr/lib
	install -m0644 milkfish/usr/lib/milkfish_functions.sh $(INSTALLDIR)/milkfish/usr/lib/

milkfish-clean:
	rm -rf $(INSTALLDIR)/milkfish/*
