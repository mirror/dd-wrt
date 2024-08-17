ipsec-tools-configure:
	make -C libsepol
	make -C libselinux
	cd ipsec-tools/flex && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS)"
	make -C ipsec-tools/flex libfl.a
	cd ipsec-tools && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" --prefix=/usr --with-kernel-headers=$(LINUXDIR)/include --with-openssl=$(SSLPATH) --with-flex=$(TOP)/ipsec-tools/flex --with-flexlib=$(TOP)/ipsec-tools/flex/libfl.a


ipsec-tools-install:
	make  -C ipsec-tools install DESTDIR=$(INSTALLDIR)/ethtool
#	rm -rf $(INSTALLDIR)/ipsec-tools/usr/man
#	rm -rf $(INSTALLDIR)/ipsec-tools/usr/lib

