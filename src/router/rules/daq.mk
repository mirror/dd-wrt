ifeq ($(CONFIG_IPV6),y)
daq-configure: libpcap libdnet-configure libnetfilter_queue-configure libdnet libnetfilter_queue iptables-new
	cd daq && autoconf
	export ac_cv_header_linux_netfilter_h=yes ; \
	export ac_cv_lib_pcap_pcap_lib_version=yes ; \
	export daq_cv_libpcap_version_1x=yes ; \
	cd daq && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--disable-ipfw-module \
		--enable-nfq-module \
		--enable-ipq-module \
		--enable-pcap-module \
		--enable-static \
		--prefix=$(TOP)/daq/install \
		--libdir=$(TOP)/daq/install/lib \
		--with-libpcap-includes="$(TOP)/libpcap" \
		--with-libpcap-libraries="$(TOP)/libpcap" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -I$(TOP)/iptables-new/include -I$(TOP)/iptables-new/include/libipq -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE  -Drpl_malloc=malloc -I$(TOP)/iptables-new/include -I$(TOP)/iptables-new/include/libipq -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include" \
		LDFLAGS="-L$(TOP)/iptables-new/libipq/.libs -L$(TOP)/libnetfilter_queue/src/.libs -L$(TOP)/libnfnetlink/src/.libs -fPIC"
	$(MAKE) -C daq all install CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -Drpl_malloc=malloc"
else
daq-configure: libpcap libdnet-configure libnetfilter_queue-configure libdnet libnetfilter_queue iptables
	cd daq && autoconf
	export ac_cv_header_linux_netfilter_h=yes ; \
	export ac_cv_lib_pcap_pcap_lib_version=yes ; \
	export daq_cv_libpcap_version_1x=yes ; \
	cd daq && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--disable-ipfw-module \
		--enable-nfq-module \
		--enable-ipq-module \
		--enable-pcap-module \
		--enable-static \
		--prefix=$(TOP)/daq/install \
		--libdir=$(TOP)/daq/install/lib \
		--with-libpcap-includes="$(TOP)/libpcap" \
		--with-libpcap-libraries="$(TOP)/libpcap" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include" \
		CPPFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE  -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include" \
		LDFLAGS="-L$(TOP)/iptables/libipq -L$(TOP)/libnetfilter_queue/src/.libs -L$(TOP)/libnfnetlink/src/.libs -fPIC"
	$(MAKE) -C daq all install CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -Drpl_malloc=malloc"
endif
daq: libpcap libdnet iptables
	-mkdir daq/install
	-rm -rf daq/install/*
	$(MAKE) -C daq all install CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -Drpl_malloc=malloc"

daq-clean:
	-rm -rf daq/install/*
	$(MAKE) -C daq clean  CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -Drpl_malloc=malloc"

daq-install:
	-install -D daq/install/lib/libsfbpf.so.0 $(INSTALLDIR)/daq/usr/lib/libsfbpf.so.0
	-install -D daq/install/lib/daq/daq_ipq.so $(INSTALLDIR)/daq/usr/lib/daq/daq_ipq.so
	-install -D daq/install/lib/daq/daq_nfq.so $(INSTALLDIR)/daq/usr/lib/daq/daq_nfq.so
	-install -D daq/install/lib/daq/daq_pcap.so $(INSTALLDIR)/daq/usr/lib/daq/daq_pcap.so
	-install -D daq/install/lib64/libsfbpf.so.0 $(INSTALLDIR)/daq/usr/lib/libsfbpf.so.0
	-install -D daq/install/lib64/daq/daq_ipq.so $(INSTALLDIR)/daq/usr/lib/daq/daq_ipq.so
	-install -D daq/install/lib64/daq/daq_nfq.so $(INSTALLDIR)/daq/usr/lib/daq/daq_nfq.so
	-install -D daq/install/lib64/daq/daq_pcap.so $(INSTALLDIR)/daq/usr/lib/daq/daq_pcap.so
