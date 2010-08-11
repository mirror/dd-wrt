relayd:
ifeq ($(CONFIG_RELAYD),y)
	$(MAKE) -C relayd
else
	@true
endif

relayd-clean:
	$(MAKE) -C relayd clean

relayd-install:
ifeq ($(CONFIG_RELAYD),y)
	install -D relayd/relayd $(INSTALLDIR)/relayd/usr/sbin/relayd
	$(STRIP) $(INSTALLDIR)/relayd/usr/sbin/relayd
else
        # So that generic rule does not take precedence
	@true
endif
