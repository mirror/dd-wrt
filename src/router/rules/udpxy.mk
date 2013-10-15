
udpxy-configure:
	@true

udpxy:
	$(MAKE) -C udpxy CFLAGS="$(COPTS) -DNEED_PRINTF" lean

udpxy-clean:
	$(MAKE) -C udpxy clean

udpxy-install:
	mkdir -p $(INSTALLDIR)/udpxy/usr/sbin
	install -D udpxy/udpxy $(INSTALLDIR)/udpxy/usr/sbin/udpxy
#	install -D udpxy/udpxrec $(INSTALLDIR)/udpxy/usr/sbin/udpxrec
	strip --strip-unneeded --remove-section=.comment --remove-section=.pdr --remove-section=.mdebug.abi32 $(INSTALLDIR)/udpxy/usr/sbin/*
