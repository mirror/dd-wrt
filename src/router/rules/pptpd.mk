pptpd-configure:
ifeq ($(CONFIG_PPTP_ACCEL),y)
	cd pptpd-accel && ./configure CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF" LDFLAGS="$(COPTS)" --prefix=/usr --with-bcrelay --host=$(ARCH)-linux
else
	cd pptpd && ./configure  --enable-bcrelay  CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/pppd/include -DNEED_PRINTF" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/pppd.new/include" LDFLAGS="$(COPTS)" --prefix=/usr --host=$(ARCH)-linux XTRALIBS_CTRL="-L$(TOP)/libutils -L$(TOP)/nvram -lshutils -lnvram"
endif
	@true

pptpd:
ifeq ($(CONFIG_PPTPD),y)
ifeq ($(CONFIG_PPTP_ACCEL),y)
	$(MAKE) -C pptpd-accel
else
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections   -I$(TOP)/pppd/include -DNEED_PRINTF" \
	CPPFLAGS="$(COPTS)$(MIPS16_OPT) $(LTO) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections   -I$(TOP)/pppd/include"  LDFLAGS="$(COPTS)" \
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
