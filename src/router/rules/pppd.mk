pppd-symlinks:
	- (cd pppd.new ; \
	ln -s linux/Makefile.top Makefile ; \
	ln -s Makefile.linux pppd.new/Makefile ; \
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
	$(MAKE) HAVE_INET6=y -j 4 -C pppd.new/pppd
else
	$(MAKE) -j 4 -C pppd.new/pppd
endif
ifeq ($(CONFIG_3G),y)
	$(MAKE) -j 4 -C pppd.new/chat
endif
ifeq ($(CONFIG_PPPSTATS),y)
	$(MAKE) -j 4 -C pppd.new/pppstats
endif
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/rp-pppoe
ifeq ($(CONFIG_PPTP_ACCEL),y)
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/pptp
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/radius
endif
ifeq ($(CONFIG_PPPOATM),y)
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/pppoatm
endif
ifeq ($(CONFIG_L2TP),y)
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/pppol2tp
endif


pppd-clean pppd-distclean: pppd-symlinks
	$(MAKE) -C pppd.new/pppd/plugins/rp-pppoe clean
	$(MAKE) -C pppd.new/pppd/plugins/pppoatm clean
	$(MAKE) -C pppd.new/pppd/plugins/radius clean
	$(MAKE) -C pppd.new/pppd/plugins/pppol2tp clean
	$(MAKE) HAVE_INET6=y  -C pppd.new/pppd clean

pppd-install:
	install -D pppd.new/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_3G),y)
	install -D pppd.new/chat/chat $(INSTALLDIR)/pppd/usr/sbin/chat
endif
	install -D pppd.new/pppd/plugins/rp-pppoe/rp-pppoe.so $(INSTALLDIR)/pppd/usr/lib/rp-pppoe.so
ifeq ($(CONFIG_PPTP_ACCEL),y)
	install -D pppd.new/pppd/plugins/pptp/pptp.so $(INSTALLDIR)/pppd/usr/lib/pptp.so
endif


ifeq ($(CONFIG_PPPOATM),y)
	install -D pppd.new/pppd/plugins/pppoatm/pppoatm.so $(INSTALLDIR)/pppd/usr/lib/pppoatm.so
endif
ifeq ($(CONFIG_L2TP),y)
	install -D pppd.new/pppd/plugins/pppol2tp/pppol2tp.so $(INSTALLDIR)/pppd/usr/lib/pppol2tp.so
endif

ifeq ($(CONFIG_PPPSTATS),y)
	install -D pppd.new/pppstats/pppstats $(INSTALLDIR)/pppd/usr/sbin/pppstats
else
	rm -rf $(INSTALLDIR)/pppd/usr/sbin/pppstats
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	install -D pppd.new/pppd/plugins/radius/radius.so $(INSTALLDIR)/pppd/usr/lib/radius.so
	install -D pppd.new/pppd/plugins/radius/radattr.so $(INSTALLDIR)/pppd/usr/lib/radattr.so
	install -D pppd.new/pppd/plugins/radius/radrealms.so $(INSTALLDIR)/pppd/usr/lib/radrealms.so
	install -d $(INSTALLDIR)/pppd/etc
	cp pppd.new/pppd/plugins/radius/etc/issue $(INSTALLDIR)/pppd/etc/issue
	cp pppd.new/pppd/plugins/radius/etc/port-id-map $(INSTALLDIR)/pppd/etc/port-id-map
	cp pppd.new/pppd/plugins/radius/etc/dictionary $(INSTALLDIR)/pppd/etc/dictionary
	cp pppd.new/pppd/plugins/radius/etc/dictionary.microsoft $(INSTALLDIR)/pppd/etc/dictionary.microsoft
	chmod 0644 $(INSTALLDIR)/pppd/etc/*
else
	rm -rf $(INSTALLDIR)/pppd/usr/lib/pppd/rad*.so
	rm -rf $(INSTALLDIR)/pppd/etc/radiusclient
endif
