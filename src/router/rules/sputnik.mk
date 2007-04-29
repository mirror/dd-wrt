sputnik:
	if test -e "sputnik/Makefile"; then make -C sputnik; fi

sputnik-clean:
	if test -e "sputnik/Makefile"; then make -C sputnik clean; fi

sputnik-install:
	mkdir -p $(INSTALLDIR)/sputnik/etc/config
	cp -fpR sputnik/config/* $(INSTALLDIR)/sputnik/etc/config
	install -D sputnik/$(ARCH)/sputnik $(INSTALLDIR)/sputnik/usr/sbin/sputnik
	install -D sputnik/$(ARCH)/libiksemel.so $(INSTALLDIR)/sputnik/usr/lib/libiksemel.so