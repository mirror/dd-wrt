quagga-configure:
	cd quagga && ./configure --host=$(ARCH)-uclibc-linux $(CONFIG_QUAGGA_EXTRA) --enable-opaque-lsa --enable-ospf-te --disable-ospfclient --enable-multipath=32  --disable-ipv6 --prefix=/usr  --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi

quagga:
	$(MAKE) -C quagga

quagga-install:
	make -C quagga DESTDIR=$(INSTALLDIR)/quagga install
	rm -rf $(INSTALLDIR)/quagga/usr/info
	rm -rf $(INSTALLDIR)/quagga/usr/include
	rm -rf $(INSTALLDIR)/quagga/usr/etc
	rm -rf $(INSTALLDIR)/quagga/usr/man
	rm -f $(INSTALLDIR)/quagga/usr/lib/*.a
	rm -f $(INSTALLDIR)/quagga/usr/lib/*.la

quagga-clean:
	if test -e "quagga/Makefile"; then $(MAKE) -C quagga clean; fi
