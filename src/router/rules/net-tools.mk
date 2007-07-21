net-tools:
	make -C net-tools
	make -C net-tools arp mii-tool
	

net-tools-install:
	mkdir -p $(INSTALLDIR)/net-tools/usr/sbin
	cp net-tools/mii-tool $(INSTALLDIR)/net-tools/usr/sbin

