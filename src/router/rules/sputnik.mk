sputnik:
	if test -e "sputnik/Makefile"; then make -C sputnik; fi

sputnik-clean:
	if test -e "sputnik/Makefile"; then make -C sputnik clean; fi

sputnik-install:
	mkdir -p $(INSTALLDIR)/sputnik/etc/config
ifeq ($(CONFIG_SPUTNIK_PRO),y)
	cp -fpR sputnik/config/1sputnik.webhotspot_pro $(INSTALLDIR)/sputnik/etc/config/1sputnik.webhotspot
else
	cp -fpR sputnik/config/1sputnik.webhotspot $(INSTALLDIR)/sputnik/etc/config/1sputnik.webhotspot
endif
	install -D sputnik/$(ARCH)/sputnik $(INSTALLDIR)/sputnik/usr/sbin/sputnik
	install -D sputnik/$(ARCH)/libiksemel.so $(INSTALLDIR)/sputnik/usr/lib/libiksemel.so