arpalert-configure:
	cd arpalert && ac_cv_header_sys_sysctl_h=yes ./configure --prefix=/usr sysconfdir=/etc --localstatedir=/var --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I../libpcap -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L$(TOP)/libpcap -lpcap"

arpalert: 
	$(MAKE) -C arpalert LDFLAGS+="-L$(TOP)/libpcap"

arpalert-clean: 
	if test -e "nmap/Makefile"; then $(MAKE) -C arpalert clean ; fi

arpalert-install: 
	$(MAKE) -C arpalert install DESTDIR=$(INSTALLDIR)/arpalert
	rm -rf $(INSTALLDIR)/arpalert/usr/share
	rm -rf $(INSTALLDIR)/arpalert/usr/include
	rm -rf $(INSTALLDIR)/arpalert/var
	rm -rf $(INSTALLDIR)/arpalert/etc/arpalert/arpalert.conf*
	rm -rf $(INSTALLDIR)/arpalert/etc/arpalert/maclist.*
	ln -sf /tmp/arpalert.conf $(INSTALLDIR)/arpalert/etc/arpalert/arpalert.conf
	ln -sf /tmp/maclist.allow $(INSTALLDIR)/arpalert/etc/arpalert/maclist.allow
	ln -sf /tmp/maclist.deny $(INSTALLDIR)/arpalert/etc/arpalert/maclist.deny
