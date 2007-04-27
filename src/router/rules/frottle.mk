frottle:
ifeq ($(CONFIG_FROTTLE),y)
	$(MAKE) -C frottle LIBDIR=/opt/openwrt/lib/
else
	@true
endif

frottle-clean:
	$(MAKE) -C frottle clean

frottle-install:
ifeq ($(CONFIG_FROTTLE),y)
	install -D frottle/frottle $(INSTALLDIR)/frottle/usr/sbin/frottle
	$(STRIP) $(INSTALLDIR)/frottle/usr/sbin/frottle
else
        # So that generic rule does not take precedence
	@true
endif