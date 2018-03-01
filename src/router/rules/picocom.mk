picocom-configure: 
	@true

picocom: 
ifeq ($(CONFIG_DIST),"micro")
	$(MAKE) -C picocom CFLAGS="-DNEED_PRINTF -DNO_HELP $(COPTS) $(MIPS16_OPT)"
else
	$(MAKE) -C picocom CFLAGS="-DNEED_PRINTF $(COPTS) $(MIPS16_OPT)"
endif

picocom-install:
	install -D picocom/picocom $(INSTALLDIR)/picocom/usr/bin/picocom

picocom-clean:
	$(MAKE) -C picocom clean CFLAGS="-DNEED_PRINTF $(COPTS) $(MIPS16_OPT)"
