http-redirect:
	@true

http-redirect-clean:
	@true

http-redirect-install:
	install -D http-redirect/http-redirect.nvramconfig $(INSTALLDIR)/http-redirect/etc/config/http-redirect.nvramconfig
	install -D http-redirect/http-redirect.webhotspot $(INSTALLDIR)/http-redirect/etc/config/http-redirect.webhotspot
	install -D http-redirect/http-redirect.firewall $(INSTALLDIR)/http-redirect/etc/config/http-redirect.firewall


