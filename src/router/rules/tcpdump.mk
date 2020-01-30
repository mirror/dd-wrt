tcpdump-configure: libpcap
	cd tcpdump && ./configure --host=$(ARCH)-linux --enable-shared --enable-ipv6 --libdir=/lib --disable-static --with-crypto=$(TOP)/openssl CC="$(CC)" ac_cv_linux_vers=2 ac_cv_ssleay_path=no td_cv_buggygetaddrinfo=no CPPFLAGS="-I../libpcap $(COPTS) $(MIPS16_OPT) $(LTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE -DNEED_PRINTF" CFLAGS="-I../libpcap $(COPTS) $(LTO) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -DHAVE_BPF_DUMP" LDFLAGS="-L../libpcap $(LDLTO)" \
		AR="$(ARCH)-linux-ar cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

tcpdump:
	make -j 4 -C tcpdump


tcpdump-install:
	install -D tcpdump/tcpdump $(INSTALLDIR)/tcpdump/usr/sbin/tcpdump

