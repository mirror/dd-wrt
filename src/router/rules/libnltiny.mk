libnltiny:
	$(MAKE) CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" -C libnl-tiny 


libnltiny-clean:
	$(MAKE) -C libnl-tiny clean
	@true

libnltiny-install:
	install -D libnl-tiny/libnl-tiny.so $(INSTALLDIR)/libnltiny/usr/lib/libnl-tiny.so
