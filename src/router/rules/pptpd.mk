pptpd-configure:
ifeq ($(CONFIG_PPTP_ACCEL),y)
	cd pptpd-accel && ./configure CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" --prefix=/usr --host=$(ARCH)-linux

endif
	@true

pptpd:
ifeq ($(CONFIG_PPTPD),y)
ifeq ($(CONFIG_PPTP_ACCEL),y)
	$(MAKE) -C pptpd-accel
else
	$(MAKE) -C pptpd
endif
else
	@true
endif

pptpd-clean:
ifeq ($(CONFIG_PPTP_ACCEL),y)
	$(MAKE) -C pptpd-accel clean
else
	$(MAKE)  -C pptpd clean
endif

pptpd-install:
ifeq ($(CONFIG_PPTPD),y)
ifeq ($(CONFIG_PPTP_ACCEL),y)
	install -D pptpd-accel/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	install -D pptpd-accel/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	install -D pptpd-accel/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	@true
else
	install -D pptpd/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	install -D pptpd/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	install -D pptpd/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
        # So that generic rule does not take precedence
	@true
endif
endif