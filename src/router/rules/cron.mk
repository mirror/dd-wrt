cron-install:
	#install -d $(INSTALLDIR)/cron/etc/cron.d
	install -D cron/cron $(INSTALLDIR)/cron/usr/sbin/cron
	$(STRIP) $(INSTALLDIR)/cron/usr/sbin/cron

