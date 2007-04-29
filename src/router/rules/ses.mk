ses:
	[ ! -f ses/Makefile ] || $(MAKE) -C ses

ses-install:
	install -D ses/config/ses.webservices $(INSTALLDIR)/ses/etc/config/ses.webservices
	install -D ses/config/ses.nvramconfig $(INSTALLDIR)/ses/etc/config/ses.nvramconfig	
#	install -D ses/ses/ses $(INSTALLDIR)/ses/usr/sbin/ses
##	$(STRIP) $(INSTALLDIR)/ses/usr/sbin/ses
