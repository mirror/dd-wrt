radauth:
	$(MAKE) -C radauth

radauth-install:
	install -D radauth/wrt-radauth $(INSTALLDIR)/radauth/usr/sbin/wrt-radauth
	install -D radauth/radius-client $(INSTALLDIR)/radauth/usr/sbin/radius-client
	install -D radauth/radiusallow $(INSTALLDIR)/radauth/usr/sbin/radiusallow
	install -D radauth/radiusdisallow $(INSTALLDIR)/radauth/usr/sbin/radiusdisallow
	install -D radauth/macupd $(INSTALLDIR)/radauth/usr/sbin/macupd
	
radauth-clean:
	$(MAKE) -C radauth clean
	