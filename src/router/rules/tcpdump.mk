ifeq ($(CONFIG_IPV6),y)
export EXTRA:=--enable-ipv6
endif



tcpdump-configure: libpcap
	cd tcpdump && ./configure --host=$(ARCH)-linux --enable-shared --disable-static --without-crypto $(EXTRA) CC="$(CC)" ac_cv_linux_vers=2 ac_cv_ssleay_path=no td_cv_buggygetaddrinfo=no CPPFLAGS="-I../libpcap_noring $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" CFLAGS="-I../libpcap_noring $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -DHAVE_BPF_DUMP" LDFLAGS="-L../libpcap_noring"

tcpdump:
	make -j 4 -C tcpdump


tcpdump-install:
	install -D tcpdump/tcpdump $(INSTALLDIR)/tcpdump/usr/sbin/tcpdump

