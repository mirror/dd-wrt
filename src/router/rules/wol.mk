wol:
	cd wol && ./configure --disable-nls --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS)"
	$(MAKE) -C wol

wol-clean:
	$(MAKE) -C wol clean

wol-install:
	install -D wol/src/wol $(INSTALLDIR)/wol/usr/sbin/wol
	$(STRIP) $(INSTALLDIR)/wol/usr/sbin/wol