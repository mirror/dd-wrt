opennds-configure:
	@true

opennds: libmicrohttpd
	install -D opennds/config/nodog.webhotspot httpd/ej_temp/nodog.webhotspot
	$(MAKE) -C opennds \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -I$(TOP)/libmicrohttpd/src/include" \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -L$(TOP)/libmicrohttpd/src/microhttpd/.libs"


opennds-install:
	$(MAKE) -C opennds install DESTDIR=$(INSTALLDIR)/opennds
	install -D opennds/config/nodog.webhotspot $(INSTALLDIR)/opennds/etc/config/nodog.webhotspot
	install -D opennds/config/nodog.nvramconfig $(INSTALLDIR)/opennds/etc/config/nodog.nvramconfig
	install -D opennds/config/nodog.startup $(INSTALLDIR)/opennds/etc/config/nodog.startup

