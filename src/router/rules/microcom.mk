microcom-clean:
	$(MAKE) -C microcom102 clean


microcom:
	$(MAKE) -C microcom102 microcom CC=$(CC)  
	$(STRIP) $(INSTALLDIR)/microcom/usr/sbin/microcom

microcom-install:
	install -D microcom102/microcom $(INSTALLDIR)/microcom/usr/sbin/microcom

