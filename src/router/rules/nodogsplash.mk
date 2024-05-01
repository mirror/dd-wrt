
nodogsplash-configure:
	@true

nodogsplash: libmicrohttpd
	install -D nodogsplash/config/nodog.webhotspot httpd/ej_temp/nodog.webhotspot
	$(MAKE) -C nodogsplash \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -I$(TOP)/libmicrohttpd/src/include" \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -L$(TOP)/libmicrohttpd/src/microhttpd/.libs"


nodogsplash-install:
	$(MAKE) -C nodogsplash install DESTDIR=$(INSTALLDIR)/nodogsplash
	install -D nodogsplash/config/nodog.webhotspot $(INSTALLDIR)/nodogsplash/etc/config/nodog.webhotspot
	install -D nodogsplash/config/nodog.nvramconfig $(INSTALLDIR)/nodogsplash/etc/config/nodog.nvramconfig
	install -D nodogsplash/config/nodog.startup $(INSTALLDIR)/nodogsplash/etc/config/nodog.startup

