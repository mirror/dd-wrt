pppd-symlinks:
	- (cd pppd ; \
	ln -s linux/Makefile.top Makefile ; \
	ln -s Makefile.linux pppd/Makefile ; \
	ln -s Makefile.linux chat/Makefile ; \
	ln -s Makefile.linux pppd/plugins/Makefile ; \
	ln -s Makefile.linux pppd/plugins/radius/Makefile ; \
	ln -s Makefile.linux pppd/plugins/rp-pppoe/Makefile ; \
	ln -s Makefile.linux pppd/plugins/pppoatm/Makefile ; \
	ln -s Makefile.linux pppdump/Makefile ; \
	ln -s Makefile.linux pppstats/Makefile ; \
	)

ifeq ($(CONFIG_PPPOATM),y)
pppd: pppd-symlinks atm
else
pppd: pppd-symlinks
endif
ifeq ($(CONFIG_IPV6),y)
	$(MAKE) HAVE_INET6=y -C pppd/pppd
else
	$(MAKE) -C pppd/pppd
endif
ifeq ($(CONFIG_3G),y)
	$(MAKE) -C pppd/chat
endif
ifeq ($(CONFIG_PPPSTATS),y)
	$(MAKE) -C pppd/pppstats
endif
	$(MAKE) -C pppd/pppd/plugins/rp-pppoe
ifeq ($(CONFIG_PPTP_ACCEL),y)
	$(MAKE) -C pppd/pppd/plugins/pptp
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	$(MAKE) -C pppd/pppd/plugins/radius
endif
ifeq ($(CONFIG_PPPOATM),y)
	$(MAKE) -C pppd/pppd/plugins/pppoatm
endif
ifeq ($(CONFIG_L2TP),y)
	$(MAKE) -C pppd/pppd/plugins/pppol2tp
endif


pppd-clean pppd-distclean: pppd-symlinks
	$(MAKE) -C pppd/pppd/plugins/rp-pppoe clean
	$(MAKE) -C pppd/pppd/plugins/pppoatm clean
	$(MAKE) -C pppd/pppd/plugins/radius clean
	$(MAKE) -C pppd/pppd/plugins/pppol2tp clean
	$(MAKE) -C pppd/chat clean
	$(MAKE) HAVE_INET6=y  -C pppd/pppd clean

pppd-install:
	install -D pppd/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_3G),y)
	install -D pppd/chat/chat $(INSTALLDIR)/pppd/usr/sbin/chat
endif
	install -D pppd/pppd/plugins/rp-pppoe/rp-pppoe.so $(INSTALLDIR)/pppd/usr/lib/rp-pppoe.so
ifeq ($(CONFIG_PPTP_ACCEL),y)
	install -D pppd/pppd/plugins/pptp/pptp.so $(INSTALLDIR)/pppd/usr/lib/pptp.so
endif


ifeq ($(CONFIG_PPPOATM),y)
	install -D pppd/pppd/plugins/pppoatm/pppoatm.so $(INSTALLDIR)/pppd/usr/lib/pppoatm.so
endif
ifeq ($(CONFIG_L2TP),y)
	install -D pppd/pppd/plugins/pppol2tp/pppol2tp.so $(INSTALLDIR)/pppd/usr/lib/pppol2tp.so
endif

ifeq ($(CONFIG_PPPSTATS),y)
	install -D pppd/pppstats/pppstats $(INSTALLDIR)/pppd/usr/sbin/pppstats
else
	rm -rf $(INSTALLDIR)/pppd/usr/sbin/pppstats
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	install -D pppd/pppd/plugins/radius/radius.so $(INSTALLDIR)/pppd/usr/lib/radius.so
	install -D pppd/pppd/plugins/radius/radattr.so $(INSTALLDIR)/pppd/usr/lib/radattr.so
	install -D pppd/pppd/plugins/radius/radrealms.so $(INSTALLDIR)/pppd/usr/lib/radrealms.so
	install -d $(INSTALLDIR)/pppd/etc
	cp pppd/pppd/plugins/radius/etc/issue $(INSTALLDIR)/pppd/etc/issue
	cp pppd/pppd/plugins/radius/etc/port-id-map $(INSTALLDIR)/pppd/etc/port-id-map
	cp pppd/pppd/plugins/radius/etc/dictionary $(INSTALLDIR)/pppd/etc/dictionary
	cp pppd/pppd/plugins/radius/etc/dictionary.microsoft $(INSTALLDIR)/pppd/etc/dictionary.microsoft
	chmod 0644 $(INSTALLDIR)/pppd/etc/*
else
	rm -rf $(INSTALLDIR)/pppd/usr/lib/pppd/rad*.so
	rm -rf $(INSTALLDIR)/pppd/etc/radiusclient
endif
