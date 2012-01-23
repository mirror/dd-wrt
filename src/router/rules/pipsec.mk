pipsec:
ifeq ($(CONFIG_PIPSEC),y)
	$(MAKE) -C pipsec
else
	@true
endif

pipsec-clean:
	$(MAKE) -C pipsec clean

pipsec-install:
ifeq ($(CONFIG_PIPSEC),y)
	install -D pipsec/pipsecd $(INSTALLDIR)/pipsec/usr/sbin/pipsecd
	$(STRIP) $(INSTALLDIR)/pipsec/usr/sbin/pipsecd
else
        # So that generic rule does not take precedence
	@true
endif
