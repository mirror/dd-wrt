
wgets-configure:
	@true

wgets:
	$(MAKE) -C wgets CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/wgets/include -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L$(TOP)/wgets/library -lxyssl" all

wgets-clean:
	$(MAKE) -C wgets clean

wgets-install:
	install -D wgets/wgets $(INSTALLDIR)/wgets/usr/sbin/wgets
