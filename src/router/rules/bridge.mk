bridge:
	$(MAKE) -C bridge brctl/brctl

bridge-clean:
	@echo "Cleaning bridge"
	$(MAKE) -C bridge clean

bridge-install:
	install -D bridge/brctl/brctl $(INSTALLDIR)/bridge/usr/sbin/brctl
	$(STRIP) $(INSTALLDIR)/bridge/usr/sbin/brctl

