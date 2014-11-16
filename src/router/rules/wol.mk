wol-configure:
	cd wol && ./configure --disable-nls --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections   -Drpl_malloc=malloc" 

wol:
	$(MAKE) -C wol

wol-clean:
	$(MAKE) -C wol clean

wol-install:
	install -D wol/src/wol $(INSTALLDIR)/wol/usr/sbin/wol
	$(STRIP) $(INSTALLDIR)/wol/usr/sbin/wol