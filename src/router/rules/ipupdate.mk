ipupdate-install:
	install -D ipupdate/ez-ipupdate $(INSTALLDIR)/ipupdate/usr/sbin/ez-ipupdate
	$(STRIP) $(INSTALLDIR)/ipupdate/usr/sbin/ez-ipupdate

