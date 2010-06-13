nas-checkout:
	rm -rf $(TOP)/nas/src
	svn co svn://svn.dd-wrt.com/private/nas/src $(TOP)/nas/src

nas-update:
	svn update $(TOP)/nas/src


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
