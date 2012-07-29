proftpd-configure:
	cd proftpd && ./configure --host=$(ARCH)-linux --prefix=/tmp/proftpd --with-modules=mod_radius ac_cv_func_setpgrp_void=y ac_cv_func_setgrent_void=y
	
proftpd:
	$(MAKE) -C proftpd

proftpd-clean:
	-$(MAKE) -C proftpd clean

proftpd-install:
	install -D proftpd/proftpd $(INSTALLDIR)/proftpd/usr/sbin/proftpd
#	install -D proftpd/proftpd.conf $(INSTALLDIR)/proftpd/etc/config/proftpd.conf
#	install -D proftpd/proftpd.startup $(INSTALLDIR)/proftpd/etc/config/proftpd.startup
	install -D proftpd/config/ftp.webnas $(INSTALLDIR)/proftpd/etc/config/ftp.webnas
	install -D proftpd/config/proftpd.nvramconfig $(INSTALLDIR)/proftpd/etc/config/proftpd.nvramconfig
	$(STRIP) $(INSTALLDIR)/proftpd/usr/sbin/proftpd


