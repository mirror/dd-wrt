parprouted:
ifeq ($(CONFIG_PARPROUTED),y)
	$(MAKE) -C parprouted
else
	@true
endif

parprouted-clean:
	$(MAKE) -C parprouted clean

parprouted-install:
ifeq ($(CONFIG_PARPROUTED),y)
	install -D parprouted/parprouted $(INSTALLDIR)/parprouted/usr/sbin/parprouted
	$(STRIP) $(INSTALLDIR)/parprouted/usr/sbin/parprouted
else
        # So that generic rule does not take precedence
	@true
endif
