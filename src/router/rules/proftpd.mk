proftpd:
	$(MAKE) -C proftpd
proftpd-install:
	install -D proftpd/proftpd $(INSTALLDIR)/proftpd/usr/sbin/proftpd
	$(STRIP) $(INSTALLDIR)/proftpd/usr/sbin/proftpd
