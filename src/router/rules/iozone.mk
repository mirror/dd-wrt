iozone:
	$(MAKE) -C iozone linux

iozone-clean:
	$(MAKE) -C iozone clean

iozone-install:
	install -D iozone/iozone $(INSTALLDIR)/iozone/usr/sbin/iozone