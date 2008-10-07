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

pppd: pppd-symlinks
	$(MAKE) -j 4 -C pppd.new/pppd
ifeq ($(CONFIG_PPPD_CHAT),y)
	$(MAKE) -j 4 -C pppd.new/chat
endif
ifeq ($(CONFIG_PPPSTATS),y)
	$(MAKE) -j 4 -C pppd.new/pppstats
endif
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/rp-pppoe
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	$(MAKE) -j 4 -C pppd.new/pppd/plugins/radius
endif
#	$(MAKE) -C pppd/pppd/plugins/pppoatm


pppd-clean pppd-distclean: pppd-symlinks
	$(MAKE) -C pppd.new/pppd/plugins/rp-pppoe clean
	$(MAKE) -C pppd.new/pppd/plugins/radius clean
	$(MAKE) -C pppd.new/pppd clean

pppd-install:
	install -D pppd.new/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_PPPD_CHAT),y)
	install -D pppd.new/chat/chat $(INSTALLDIR)/pppd/usr/sbin/chat
endif
	install -D pppd.new/pppd/plugins/rp-pppoe/rp-pppoe.so $(INSTALLDIR)/pppd/usr/lib/rp-pppoe.so
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
#	rm -rf $(INSTALLDIR)/pppd/usr/lib/pppd/rp-pppoe.so

#	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_PPPSTATS),y)
	$(STRIP) $(INSTALLDIR)/pppd.new/usr/sbin/pppstats
endif
