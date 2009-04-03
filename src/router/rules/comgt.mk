
comgt:
	$(MAKE) -C comgt CC=$(CC)  
#	$(STRIP) $(INSTALLDIR)/comgt/usr/sbin/comgt

comgt-install:
	install -D comgt/comgt $(INSTALLDIR)/comgt/usr/sbin/comgt
	install -D comgt/scripts/dial.comgt $(INSTALLDIR)/etc/comgt/dial.comgt
	install -D comgt/scripts/setmode.comgt $(INSTALLDIR)/etc/comgt/setmode.comgt
#	install -D comgt/usb_modeswitch $(INSTALLDIR)/comgt/usr/sbin/usb_modeswitch

