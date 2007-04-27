squid:
	cd squid && ./configure --target=armeb-linux --host=armeb-linux --prefix=/usr
	make -C squid

squid-install:
	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	

