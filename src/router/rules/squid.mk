squid:
	cd squid && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr
	make -C squid

squid-install:
	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	

