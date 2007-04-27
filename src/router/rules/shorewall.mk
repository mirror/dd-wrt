shorewall-install:
	install -D shorewall/common.def     $(INSTALLDIR)/shorewall/usr/sbin/common.def
	install -D shorewall/firewall       $(INSTALLDIR)/shorewall/usr/sbin/firewall
	install -D shorewall/functions      $(INSTALLDIR)/shorewall/usr/sbin/functions
	install -D shorewall/shorewall      $(INSTALLDIR)/shorewall/usr/sbin/shorewall
	install -D shorewall/shorewall.conf $(INSTALLDIR)/shorewall/usr/sbin/shorewall.conf
	install -D shorewall/version        $(INSTALLDIR)/shorewall/usr/sbin/version

shorewall-clean:
	$(MAKE) -C shorewall clean

