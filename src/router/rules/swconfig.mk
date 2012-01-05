swconfig-configure:
	@true

swconfig:
	$(MAKE) -C swconfig CFLAGS="$(COPTS) -I$(LINUXDIR)/include -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

swconfig-clean:
	$(MAKE) -C swconfig clean CFLAGS="$(COPTS) -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

swconfig-install:
	install -D swconfig $(INSTALLDIR)/usr/sbin/swconfig
