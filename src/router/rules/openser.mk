openser:
	$(MAKE) -j 4 -C openser all utils/gen_ha1/gen_ha1 CFLAGS="$(COPTS)" extra_defs="-DNEED_PRINTF -DUSE_PTHREAD_MUTEX " prefix=/ ARCH=$(ARCH)

openser-install:
	install -D openser/openser $(INSTALLDIR)/openser/usr/sbin/openser
	install -D openser/utils/gen_ha1/gen_ha1 $(INSTALLDIR)/openser/usr/sbin/openser_gen_ha1
	mkdir -p $(INSTALLDIR)/openser/usr/lib/openser/modules
	cp -a $(OPENSER_MODULE_FILES) $(INSTALLDIR)/openser/usr/lib/openser/modules/
	mv $(INSTALLDIR)/openser/usr/lib/openser/modules/nathelper.so $(INSTALLDIR)/openser/usr/lib/openser/modules/milkfish_nathelper.so
	install -D openser/scripts/sc $(INSTALLDIR)/openser/usr/sbin/openserctl
	install -D openser/scripts/sc.dbtext $(INSTALLDIR)/openser/usr/sbin/dbtextctl

openser-clean:
	$(MAKE) -C openser clean
	rm -rf $(INSTALLDIR)/openser/*