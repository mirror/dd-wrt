
nas:
ifeq ($(CONFIG_MSSID),y)
	make -C nas
endif

nas-clean:
	make -C nas clean

nas-install:
ifeq ($(CONFIG_MSSID),y)
	make -C nas install
#	install -D nas/nas $(INSTALLDIR)/nas/usr/sbin/nas
else
	install -D nas/nas.v23 $(INSTALLDIR)/nas/usr/sbin/nas
endif
	$(STRIP) $(INSTALLDIR)/nas/usr/sbin/nas
	cd $(INSTALLDIR)/nas/usr/sbin && ln -sf nas nas4not && ln -sf nas nas4wds
