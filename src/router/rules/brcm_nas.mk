nas nas-clean nas-distclean:
        # So that generic rule does not take precedence
	@true

nas-install:
ifeq ($(CONFIG_MSSID),y)
	install -D nas/nas.6000 $(INSTALLDIR)/nas/usr/sbin/nas
else
	install -D nas/nas.v23 $(INSTALLDIR)/nas/usr/sbin/nas
endif
	$(STRIP) $(INSTALLDIR)/nas/usr/sbin/nas
	cd $(INSTALLDIR)/nas/usr/sbin && ln -sf nas nas4not && ln -sf nas nas4wds
