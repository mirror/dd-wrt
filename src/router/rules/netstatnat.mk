netstatnat:
	$(MAKE) -C netstatnat

netstatnat-clean:
	$(MAKE) -C netstatnat clean

netstatnat-install:
	install -D netstatnat/netstat-nat $(INSTALLDIR)/netstatnat/usr/sbin/netstat-nat
	$(STRIP) $(INSTALLDIR)/netstatnat/usr/sbin/netstat-nat

