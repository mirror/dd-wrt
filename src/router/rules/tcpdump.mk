ifeq ($(CONFIG_IPV6),y)
export EXTRA:=--enable-ipv6
endif



tcpdump-configure: libpcap
	cd tcpdump && ./configure --host=$(ARCH)-linux --enable-shared --disable-static --without-crypto $(EXTRA) CC=$(ARCH)-linux-uclibc-gcc ac_cv_linux_vers=2 td_cv_buggygetaddrinfo=no CPPFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap_noring $(COPTS) -DNEED_PRINTF -DHAVE_BPF_DUMP" LDFLAGS="-L../libpcap_noring"

tcpdump:
	make -j 4 -C tcpdump


tcpdump-install:
	install -D tcpdump/tcpdump $(INSTALLDIR)/tcpdump/usr/sbin/tcpdump

