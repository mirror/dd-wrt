gcom:
	$(MAKE) -C gcom CC=$(CC)  
	$(STRIP) $(INSTALLDIR)/gcom/usr/sbin/gcom

gcom-install:
	install -D gcom/gcom $(INSTALLDIR)/gcom/usr/sbin/gcom

