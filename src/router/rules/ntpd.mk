ntpd:
	cd ntpd && ./configure --host=armeb-linux CFLAGS="$(COPTS)" --prefix=/usr
	make   -C ntpd

ntpd-clean:
	make   -C ntpd clean

ntpd-install:
	make   -C ntpd install DESTDIR=$(INSTALLDIR)/ntpd
	rm -rf $(INSTALLDIR)/ntpd/usr/man
	rm -rf $(INSTALLDIR)/ntpd/usr/lib

