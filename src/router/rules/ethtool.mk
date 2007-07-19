ethtool:
	cd ethtool && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" --prefix=/usr
	make   -C ethtool

ethtool-clean:
	make   -C ethtool clean

ethtool-install:
	make   -C ethtool install DESTDIR=$(INSTALLDIR)/ethtool
	rm -rf $(INSTALLDIR)/ethtool/usr/man
	rm -rf $(INSTALLDIR)/ethtool/usr/lib

