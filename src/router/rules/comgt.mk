comgt-configure:
	$(MAKE) -C usb_modeswitch configure


comgt-clean:
	$(MAKE) -C usb_modeswitch clean
	$(MAKE) -C comgt clean

comgt:
	$(MAKE) -C usb_modeswitch
	$(MAKE) -C comgt CC=$(CC)  
#	$(STRIP) $(INSTALLDIR)/comgt/usr/sbin/comgt

comgt-install:
	install -D comgt/comgt $(INSTALLDIR)/comgt/usr/sbin/comgt
	install -D comgt/scripts/dial.comgt $(INSTALLDIR)/comgt/etc/comgt/dial.comgt
	install -D comgt/scripts/setmode.comgt $(INSTALLDIR)/comgt/etc/comgt/setmode.comgt
	install -D comgt/scripts/reset.comgt $(INSTALLDIR)/comgt/etc/comgt/reset.comgt
	install -D comgt/scripts/wakeup.comgt $(INSTALLDIR)/comgt/etc/comgt/wakeup.comgt
	install -D usb_modeswitch/usb_modeswitch $(INSTALLDIR)/comgt/usr/sbin/usb_modeswitch
	install -D usb_modeswitch/ozerocdoff $(INSTALLDIR)/comgt/usr/sbin/ozerocdoff
#	install -D comgt/usb_modeswitch $(INSTALLDIR)/comgt/usr/sbin/usb_modeswitch

