nprobe-configure:
	chmod +x nprobe/configure
	chmod +x nprobe/config.guess
	cd nprobe && ./configure --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CPPFLAGS="-I../libpcap $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap $(COPTS) -DNEED_PRINTF" LDFLAGS="-L../libpcap" --with-only-ipv4 PCAP_ROOT="$(TOP)/libpcap" \

nprobe:
	make -C nprobe all


nprobe-clean:
	make -j 4 -C nprobe clean


nprobe-install:
	make -C nprobe install DESTDIR=$(INSTALLDIR)/nprobe
	rm -f $(INSTALLDIR)/nprobe/usr/lib/*.la
	rm -f $(INSTALLDIR)/nprobe/usr/lib/*.a
    
