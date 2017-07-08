shownf:
	@true

shownf-clean:
	@true

shownf-install:
	install -D shownf/configs/shownf.nvramconfig $(INSTALLDIR)/shownf/etc/config/shownf.nvramconfig
	install -D shownf/configs/shownf.webconfig $(INSTALLDIR)/shownf/etc/config/shownf.webconfig
	install -D shownf/configs/00-shownf-head.shownf $(INSTALLDIR)/shownf/etc/config/00-shownf-head.shownf
	install -D shownf/configs/99-shownf.shownf $(INSTALLDIR)/shownf/etc/config/99-shownf.shownf
#
	-install -D shownf/snippets/02-dnscrypt.shownf $(INSTALLDIR)/shownf/etc/config/02-dnscrypt.shownf
