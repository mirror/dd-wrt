softflowd-configure:
	#export CFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" CPPFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" ; \
	#export CXXFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" ; 
	cd softflowd && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr CPPFLAGS="-I../libpcap $(COPTS) -DNEED_PRINTF" CFLAGS="-I../libpcap $(COPTS) -DNEED_PRINTF" LDFLAGS="-L../libpcap" PCAP_ROOT="$(TOP)/libpcap"

softflowd:
	make -C softflowd

softflowd-clean:
	if test -e "softflowd/Makefile"; then make -C softflowd clean; fi
	@true

softflowd-install:
	install -D softflowd/softflowd $(INSTALLDIR)/softflowd/usr/sbin/softflowd
	install -D softflowd/softflowctl $(INSTALLDIR)/softflowd/usr/sbin/softflowctl
