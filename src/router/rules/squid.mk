squid-configure:
	cd squid && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr

squid:
	make -C squid

squid-install:
	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	

