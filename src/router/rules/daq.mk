daq-configure: libpcap libdnet libnetfilter_queue
	cd daq && autoconf
	export ac_cv_header_linux_netfilter_h=yes ; \
	export CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include" ; \
	export LDFLAGS="-L$(TOP)/iptables/libipq -L$(TOP)/libnetfilter_queue/src/.libs -L$(TOP)/libnfnetlink/src/.libs" ;\
	cd daq && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--disable-ipfw-module \
		--enable-nfq-module \
		--enable-ipq-module \
		--enable-static \
		--prefix=$(TOP)/daq/install \
		--with-libpcap-includes="$(TOP)/libpcap_noring" \
		--with-libpcap-libraries="$(TOP)/libpcap_noring" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs"

daq: libpcap libdnet 
	-mkdir daq/install
	-rm -rf daq/install/*
	$(MAKE) -C daq install

daq-clean:
	-rm -rf daq/install/*
	$(MAKE) -C daq clean

daq-install:
	$(MAKE) -C daq install CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF"
