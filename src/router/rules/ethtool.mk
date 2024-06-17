ethtool-configure: libmnl
	cd ethtool && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF $(MIPS16_OPT) $(LTO)" MNL_CFLAGS="-I$(TOP)/libmnl/include" MNL_LIBS="-L$(TOP)/libmnl/src/.libs -lmnl" --prefix=/usr 

ethtool:
	make   -C ethtool

ethtool-clean:
	make   -C ethtool clean

ethtool-install:
	make   -C ethtool install DESTDIR=$(INSTALLDIR)/ethtool
	rm -rf $(INSTALLDIR)/ethtool/usr/share/man
	rm -rf $(INSTALLDIR)/ethtool/usr/lib

