nprobe-configure:
	cd nprobe && ./configure --host=$(ARCH)-linux CC=$(ARCH)-linux-uclibc-gcc CPPFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" LDFLAGS="-L../libpcap_noring" --with-only-ipv4 PCAP_ROOT="$(TOP)/libpcap_noring" \

nprobe:
	make -j 4 -C nprobe


nprobe-install:
	install -D nprobe/nprobe $(INSTALLDIR)/nprobe/usr/sbin/nprobe

