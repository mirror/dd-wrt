smtp-redirect:
	@true

smtp-redirect-clean:
	@true

smtp-redirect-install:
	install -D smtp-redirect/smtp-redirect.nvramconfig $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.nvramconfig
	install -D smtp-redirect/smtp-redirect.webhotspot $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.webhotspot
	install -D smtp-redirect/smtp-redirect.firewall $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.firewall
