relayd:
	$(MAKE) -C relayd

relayd-clean:
	$(MAKE) -C relayd clean

relayd-install:
	install -D relayd/relayd $(INSTALLDIR)/relayd/usr/sbin/relayd
	$(STRIP) $(INSTALLDIR)/relayd/usr/sbin/relayd
