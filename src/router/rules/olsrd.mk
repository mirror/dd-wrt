olsrd-clean:
	make -C olsrd OS=linux uberclean

olsrd:
	make -C olsrd OS=linux
	make -C olsrd OS=linux libs

olsrd-install:
	install -D olsrd/olsrd $(INSTALLDIR)/olsrd/usr/sbin/olsrd
	install -D olsrd/lib/dyn_gw_plain/olsrd_dyn_gw_plain.so.0.4 $(INSTALLDIR)/olsrd/usr/lib/olsrd_dyn_gw_plain.so
ifneq ($(CONFIG_DIST),"micro")
	install -D olsrd/lib/dyn_gw/olsrd_dyn_gw.so.0.5 $(INSTALLDIR)/olsrd/usr/lib/olsrd_dyn_gw.so
	install -D olsrd/lib/httpinfo/olsrd_httpinfo.so.0.1 $(INSTALLDIR)/olsrd/usr/lib/olsrd_httpinfo.so
	install -D olsrd/lib/nameservice/olsrd_nameservice.so.0.4 $(INSTALLDIR)/olsrd/usr/lib/olsrd_nameservice.so
	install -D olsrd/lib/secure/olsrd_secure.so.0.6 $(INSTALLDIR)/olsrd/usr/lib/olsrd_secure.so.0.6
endif
	install -D olsrd/config/olsrd.nvramconfig $(INSTALLDIR)/olsrd/etc/config/olsrd.nvramconfig
