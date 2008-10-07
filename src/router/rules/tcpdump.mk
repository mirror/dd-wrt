tcpdump-configure:
	cd tcpdump && ./configure --host=$(ARCH)-linux --enable-shared --disable-static --without-crypto CC=$(ARCH)-linux-uclibc-gcc ac_cv_linux_vers=2 CPPFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" LDFLAGS="-L../libpcap_noring"

tcpdump:
	make -j 4 -C tcpdump


tcpdump-install:
	install -D tcpdump/tcpdump $(INSTALLDIR)/tcpdump/usr/sbin/tcpdump

