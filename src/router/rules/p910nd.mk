p910nd-configure:
	true
p910nd:
	make -C p910nd

p910nd-clean:
	make -C p910nd clean

p910nd-install:
	install -D p910nd/p910nd $(INSTALLDIR)/p910nd/usr/sbin/p910nd
