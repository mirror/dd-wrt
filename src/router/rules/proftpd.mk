proftpd-configure:
	cd proftpd && ./configure --disable-nls --enable-largefile --host=$(ARCH)-linux --prefix=/tmp/proftpd --disable-trace --disable-ctrls --disable-ncurses --disable-curses --with-modules=mod_radius ac_cv_func_setpgrp_void=y ac_cv_func_setgrent_void=yes CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTOMIN) -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"
	sed -i 's/HAVE_LU/HAVE_LLU/g' proftpd/config.h

proftpd:
	install -D proftpd/config/ftp.webnas httpd/ej_temp/01ftp.webnas
	install -D filesharing/config/zfilesharing.webnas httpd/ej_temp/03zfilesharing.webnas
	$(MAKE) -C proftpd

proftpd-clean:
	-$(MAKE) -C proftpd clean

proftpd-install:
	install -D proftpd/proftpd $(INSTALLDIR)/proftpd/usr/sbin/proftpd
	install -D proftpd/config/ftp.webnas $(INSTALLDIR)/proftpd/etc/config/01ftp.webnas
	install -D proftpd/config/proftpd.nvramconfig $(INSTALLDIR)/proftpd/etc/config/proftpd.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/proftpd/etc/config/03zfilesharing.webnas
	$(STRIP) $(INSTALLDIR)/proftpd/usr/sbin/proftpd


