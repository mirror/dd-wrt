sputnik:
	make -C sputnik

sputnik-clean:
	make -C sputnik clean

sputnik-install:
	mkdir -p $(INSTALLDIR)/sputnik/etc/config
	cp -fpR sputnik/config/* $(INSTALLDIR)/sputnik/etc/config
	install -D sputnik/$(ARCH)/sputnik $(INSTALLDIR)/sputnik/usr/sbin/sputnik
	install -D sputnik/$(ARCH)/libiksemel.so $(INSTALLDIR)/sputnik/usr/lib/libiksemel.so

