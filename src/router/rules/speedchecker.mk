speedchecker:
	make DDWRT=1 -C speedchecker

speedchecker-clean:
	-make DDWRT=1 -C speedchecker clean

speedchecker-install:
	install -D speedchecker/configs/speedchecker.nvramconfig $(INSTALLDIR)/speedchecker/etc/config/speedchecker.nvramconfig
	install -D speedchecker/configs/speedchecker.webservices $(INSTALLDIR)/speedchecker/etc/config/speedchecker.webservices
	install -D speedchecker/scc $(INSTALLDIR)/speedchecker/usr/sbin/scc
	install -D speedchecker/scripts/ping $(INSTALLDIR)/speedchecker/usr/bin/speedchecker/ping
	install -D speedchecker/scripts/traceroute $(INSTALLDIR)/speedchecker/usr/bin/speedchecker/traceroute
