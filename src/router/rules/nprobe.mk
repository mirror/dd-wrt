nprobe-configure:
	chmod +x nprobe/configure
	chmod +x nprobe/config.guess
	cd nprobe && ./configure --prefix=/usr --host=$(ARCH)-linux CC=$(ARCH)-linux-uclibc-gcc CPPFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" LDFLAGS="-L../libpcap_noring" --with-only-ipv4 PCAP_ROOT="$(TOP)/libpcap_noring" \

nprobe:
	make -C nprobe all


nprobe-clean:
	make -j 4 -C nprobe clean


nprobe-install:
	make -C nprobe install DESTDIR=$(INSTALLDIR)/nprobe

