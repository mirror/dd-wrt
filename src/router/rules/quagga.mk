quagga:
	cd quagga && ./configure --host=$(ARCH)-uclibc-linux --enable-opaque-lsa --enable-ospf-te --disable-ospfclient --enable-multipath=32  --disable-ipv6 --prefix=/usr  --disable-ospf6d  --enable-user=root --enable-group=root --enable-isisd --disable-ospfapi
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
	$(MAKE) -C quagga clean
