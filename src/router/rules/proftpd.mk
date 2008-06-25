proftpd-configure:
	cd proftpd && ./configure --host=mipsel-linux --prefix=/tmp/proftpd 
	
proftpd:
	$(MAKE) -C proftpd

proftpd-clean:
	$(MAKE) -C proftpd clean

proftpd-install:
	install -D proftpd/proftpd $(INSTALLDIR)/proftpd/usr/sbin/proftpd
	install -D proftpd/proftpd.conf $(INSTALLDIR)/proftpd/etc/config/proftpd.conf
	$(STRIP) $(INSTALLDIR)/proftpd/usr/sbin/proftpd


