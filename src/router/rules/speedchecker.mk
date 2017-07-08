speedchecker:
	-make DDWRT=1 -C speedchecker clean
	make DDWRT=1 -C speedchecker

speedchecker-clean:
	-make DDWRT=1 -C speedchecker clean

speedchecker-install:
	install -D speedchecker/configs/speedchecker.nvramconfig $(INSTALLDIR)/speedchecker/etc/config/speedchecker.nvramconfig
	install -D speedchecker/configs/speedchecker.startup $(INSTALLDIR)/speedchecker/etc/config/speedchecker.startup
	install -D speedchecker/scc $(INSTALLDIR)/speedchecker/usr/sbin/scc
	install -D speedchecker/scripts/ping $(INSTALLDIR)/speedchecker/usr/bin/speedchecker/ping
	install -D speedchecker/scripts/traceroute $(INSTALLDIR)/speedchecker/usr/bin/speedchecker/traceroute
	svn log $(TOP)/private/speedchecker/scc.c | head -n2 | tail -n1 | cut -d \| -f 1 |cut -c2- | awk '{print "PSVNVERSION="$$1}' >$(INSTALLDIR)/speedchecker/usr/bin/speedchecker/sccversion
	cat $(TOP)/private/speedchecker/speedchecker/VERSION >>$(INSTALLDIR)/speedchecker/usr/bin/speedchecker/sccversion
	install -D speedchecker/configs/01-sc.shownf $(INSTALLDIR)/speedchecker/etc/config/01-sc.shownf
