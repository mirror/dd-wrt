wgets:
	$(MAKE) -C wgets

wgets:
	$(MAKE) -C wgets clean

wgets-install:
	install -D wgets/wgets $(INSTALLDIR)/wol/usr/sbin/wgets
