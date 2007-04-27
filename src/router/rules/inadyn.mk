inadyn:
	$(MAKE) -C inadyn

inadyn-install:
	install -D inadyn/bin/linux/inadyn $(INSTALLDIR)/inadyn/usr/sbin/inadyn
	$(STRIP) $(INSTALLDIR)/inadyn/usr/sbin/inadyn

inadyn-clean:
	$(MAKE) -C inadyn clean



