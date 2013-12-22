snort-configure: daq-configure pcre-configure daq pcre

	export ac_cv_func_malloc_0_nonnull=yes  ; \
	export lt_sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" ; \
	export lt_sys_lib_search_path_spec="$(ARCH)-uclibc" ; \
	export CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export CPPFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -DNEED_PRINTF  -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export LDFLAGS="-L$(TOP)/iptables/libipq -L$(TOP)/libnetfilter_queue/src/.libs $(TOP)/libnetfilter_queue/src/.libs/libnetfilter_queue.a -L$(TOP)/libnet/lib -L$(TOP)/libnfnetlink/src/.libs $(TOP)/libnfnetlink/src/.libs/libnfnetlink.so  -L$(TOP)/libdnet/src/.libs -ldnet -lipq -lnet -L$(TOP)/libpcap_noring -lpcap" ;\
	cd snort && ./configure \
		--enable-reload \
		--enable-ipv6 \
		--libdir=/tmp \
		--prefix=/usr \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--without-mysql \
		--without-postgresql \
		--enable-pthread \
		--enable-gre \
		--enable-dynamicplugin \
		--enable-react \
		--enable-active-response \
		--enable-flexresp3 \
		--with-libpcre-includes="$(TOP)/pcre" \
		--with-libpcre_libraries="$(TOP)/pcre/.libs" \
		--with-libpcap-includes="$(TOP)/libpcap_noring" \
		--with-libpcap-libraries="$(TOP)/libpcap_noring" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs" \
		--with-daq-includes="$(TOP)/daq/install/include" \
		--with-daq-libraries="$(TOP)/daq/install/lib64" \
		PATH=$(TOP)/daq/install/bin:$(PATH)

snort: pcre
	$(MAKE) -C snort CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/librpc"

snort-clean:
	$(MAKE) -C snort clean CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/librpc"

snort-install:
	$(MAKE) -C snort install DESTDIR=$(INSTALLDIR)/snort CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/librpc"
	rm -rf $(INSTALLDIR)/snort/usr/src
	rm -rf $(INSTALLDIR)/snort/usr/share
	rm -rf $(INSTALLDIR)/snort/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicengine/*.la
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicpreprocessor/*.la
