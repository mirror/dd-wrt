dlna:
        # So that generic rule does not take precedence
	@true

dlna-install:
	mkdir -p $(INSTALLDIR)/dlna/usr/lib
	mkdir -p $(INSTALLDIR)/dlna/usr/bin
	mkdir -p $(INSTALLDIR)/dlna/usr/share
	cp -dR $(TOP)/dlna/usr/lib/* $(INSTALLDIR)/dlna/usr/lib
	cp -dR $(TOP)/dlna/usr/bin/* $(INSTALLDIR)/dlna/usr/bin
	cp -dR $(TOP)/dlna/usr/share/* $(INSTALLDIR)/dlna/usr/share

