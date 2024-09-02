tcpdump-configure: libpcap
	cd tcpdump && ./configure --host=$(ARCH)-linux --enable-shared --enable-ipv6 --with-crypto=$(SSLPATH) --libdir=/lib --disable-static CC="$(CC)" ac_cv_linux_vers=2 ac_cv_ssleay_path=no td_cv_buggygetaddrinfo=no CPPFLAGS="-I$(TOP)/libpcap -I$(SSLPATH) $(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE -DNEED_PRINTF" CFLAGS=" -I$(TOP)/tcpdump -I$(TOP)/libpcap $(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -DHAVE_BPF_DUMP  -D_GNU_SOURCE" LDFLAGS="-L$(TOP)/libpcap -L$(SSLPATH) $(LDLTO)" \
		AR="$(ARCH)-linux-ar cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	sed -i 's/-I\/usr\/include/ /g' $(TOP)/tcpdump/Makefile
	sed -i 's/-Wl,-rpath,\/usr\/lib64/ /g' $(TOP)/tcpdump/Makefile
	sed -i 's/-L\/usr\/lib64/ /g' $(TOP)/tcpdump/Makefile
tcpdump:
	make -C tcpdump


tcpdump-install:
	install -D tcpdump/tcpdump $(INSTALLDIR)/tcpdump/usr/sbin/tcpdump

