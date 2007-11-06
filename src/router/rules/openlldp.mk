openlldp-configure:
	cd openlldp && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS)"

openlldp:
	$(MAKE) -C openlldp

openlldp-clean:
	if test -e "openlldp/Makefile"; then make -C openlldp clean; fi
	@true

openlldp-install:
	install -D openlldp/src/lldpd $(INSTALLDIR)/openlldr/usr/sbin/lldpd

