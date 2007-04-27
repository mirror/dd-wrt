pptpd:
ifeq ($(CONFIG_PPTPD),y)
	$(MAKE) -C pptpd
else
	@true
endif

pptpd-clean:
	$(MAKE)  -C pptpd clean

pptpd-install:
ifeq ($(CONFIG_PPTPD),y)
	install -D pptpd/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	install -D pptpd/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	install -D pptpd/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	install -D pptpd/config/pptp.webservices $(INSTALLDIR)/pptpd/etc/config/pptp.webservices
else
        # So that generic rule does not take precedence
	@true
endif