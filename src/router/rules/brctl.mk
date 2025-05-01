brctl:
	$(MAKE) -C brctl brctl/brctl

brctl-clean:
	@echo "Cleaning brctl"
	$(MAKE) -C brctl clean

brctl-install:
	install -D brctl/brctl/brctl $(INSTALLDIR)/brctl/usr/sbin/brctl
	$(STRIP) $(INSTALLDIR)/brctl/usr/sbin/brctl

