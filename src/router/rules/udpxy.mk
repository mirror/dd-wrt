
udpxy-configure:
	@true

udpxy:
	$(MAKE) -C udpxy CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" lean

udpxy-clean:
	$(MAKE) -C udpxy clean

udpxy-install:
	mkdir -p $(INSTALLDIR)/udpxy/usr/sbin
	install -D udpxy/udpxy $(INSTALLDIR)/udpxy/usr/sbin/udpxy
#	install -D udpxy/udpxrec $(INSTALLDIR)/udpxy/usr/sbin/udpxrec
