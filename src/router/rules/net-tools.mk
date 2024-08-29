net-tools-clean:
	make -C net-tools clean
	@rm -f net-tools/mii-tool

net-tools:
	make -C net-tools
	make -C net-tools arp mii-tool netstat
	

net-tools-install:
	mkdir -p $(INSTALLDIR)/net-tools/usr/sbin
	cp net-tools/mii-tool $(INSTALLDIR)/net-tools/usr/sbin
	cp net-tools/netstat $(INSTALLDIR)/net-tools/usr/sbin

