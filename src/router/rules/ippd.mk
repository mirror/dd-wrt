ippd:
	$(MAKE) -C ippd
ippd-install:
	install -D ippd/ippd $(INSTALLDIR)/ippd/usr/sbin/ippd
	$(STRIP) $(INSTALLDIR)/ippd/usr/sbin/ippd