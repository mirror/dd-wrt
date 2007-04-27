ses:
	[ ! -f ses/Makefile ] || $(MAKE) -C ses

ses-install:
	install -D ses/ses/ses $(INSTALLDIR)/ses/usr/sbin/ses
	$(STRIP) $(INSTALLDIR)/ses/usr/sbin/ses
