udhcpd-clean:
	make -C udhcpc clean
	make -C udhcpd clean
	
udhcpd: shared nvram
	make -C udhcpc
	make -C udhcpd

udhcpd-install:
	install -D udhcpd/udhcpd $(INSTALLDIR)/udhcpd/usr/sbin/udhcpd
	install -D udhcpd/dumpleases $(INSTALLDIR)/udhcpd/usr/sbin/dumpleases
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/udhcpd/etc/config/dhcpd.webservices
	install -D udhcpd/config/dhcpd.startup $(INSTALLDIR)/udhcpd/etc/config/dhcpd.startup
	$(STRIP) $(INSTALLDIR)/udhcpd/usr/sbin/udhcpd
	$(STRIP) $(INSTALLDIR)/udhcpd/usr/sbin/dumpleases
	install -D udhcpc/udhcpc $(INSTALLDIR)/udhcpd/usr/sbin/udhcpc

