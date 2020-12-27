udhcpd-clean:
	make -C udhcpc clean
	make -C udhcpd clean
	
udhcpd: shared nvram
ifeq ($(CONFIG_UDHCPD),y)
	install -D udhcpd/config/dhcpd.webservices httpd/ej_temp/dhcpd.webservices
endif
	make -C udhcpc
	make -C udhcpd

udhcpd-install:
ifeq ($(CONFIG_UDHCPD),y)
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/udhcpd/etc/config/dhcpd.webservices
endif
	install -m 777 -D udhcpc/scripts/udhcpcroutes.sh $(INSTALLDIR)/udhcpd/etc/cidrroute.sh
	#mkdir -p $(INSTALLDIR)/udhcpc/etc/
	#cd $(INSTALLDIR)/udhcpd/etc/ && ln -sf /tmp/cidrroute.sh cidrroute.sh

