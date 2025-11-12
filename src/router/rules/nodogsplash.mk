
nodogsplash-configure:
	@true

nodogsplash: libmicrohttpd json-c
	install -D nodogsplash/config/nodog.webhotspot httpd/ej_temp/nodog.webhotspot
	$(MAKE) -C nodogsplash

nodogsplash-install:
	$(MAKE) -C nodogsplash install DESTDIR=$(INSTALLDIR)/nodogsplash
	install -D nodogsplash/config/nodog.webhotspot $(INSTALLDIR)/nodogsplash/etc/config/nodog.webhotspot
	install -D nodogsplash/config/nodog.nvramconfig $(INSTALLDIR)/nodogsplash/etc/config/nodog.nvramconfig
	install -D nodogsplash/config/nodog.startup $(INSTALLDIR)/nodogsplash/etc/config/nodog.startup

