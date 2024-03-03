ifneq ($(CONFIG_IPV6),y)
snort-configure: libnet daq-configure pcre-configure daq pcre zlib
	cd snort && autoreconf
	export ac_cv_func_malloc_0_nonnull=yes  ; \
	export have_inaddr_none=yes ; \
	export lt_sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" ; \
	export lt_sys_lib_search_path_spec="$(ARCH)-uclibc" ; \
	export CFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/zlib -I$(TOP)/iptables/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export CPPFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -fPIC -DNEED_PRINTF  -D_GNU_SOURCE -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables/include -I$(TOP)/zlib -I$(TOP)/iptables/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export LDFLAGS="-L$(TOP)/iptables/libipq/ -L$(TOP)/libnetfilter_queue/src/.libs $(TOP)/libnetfilter_queue/src/.libs/libnetfilter_queue.a -L$(TOP)/libnet/lib -L$(TOP)/zlib -L$(TOP)/libnfnetlink/src/.libs $(TOP)/libnfnetlink/src/.libs/libnfnetlink.so  -L$(TOP)/libdnet/src/.libs -ldnet -lipq -lnet -L$(TOP)/libpcap -lpcap" ;\
	cd snort && ./configure \
		--enable-reload \
		--enable-ipv6 \
		--libdir=/usr/lib \
		--prefix=/usr \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--without-mysql \
		--without-postgresql \
		--disable-open-appid \
		--enable-pthread \
		--enable-gre \
		--enable-dynamicplugin \
		--enable-react \
		--enable-active-response \
		--enable-flexresp3 \
		--enable-fast-install=yes \
		--with-libpcre-includes="$(TOP)/pcre" \
		--with-libpcre_libraries="$(TOP)/pcre/.libs" \
		--with-libpcap-includes="$(TOP)/libpcap" \
		--with-libpcap-libraries="$(TOP)/libpcap" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs" \
		--with-daq-includes="$(TOP)/daq/install/include" \
		--with-daq-libraries="$(TOP)/daq/install/lib" \
		--with-lzma-includes="$(TOP)/xz/src/liblzma/api" \
		--with-lzma-libraries="$(TOP)/xz/src/liblzma/.libs" \
		PATH=$(TOP)/daq/install/bin:$(PATH)
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/src/output-plugins/Makefile
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/src/Makefile
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/tools/u2boat/Makefile
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/snort/ltmain.sh
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/snort/libtool
else
snort-configure: libnet daq-configure pcre-configure daq pcre zlib
	cd snort && autoreconf
	export ac_cv_func_malloc_0_nonnull=yes  ; \
	export have_inaddr_none=yes ; \
	export lt_sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" ; \
	export lt_sys_lib_search_path_spec="$(ARCH)-uclibc" ; \
	export CFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -fPIC -DNEED_PRINTF -D_GNU_SOURCE -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables-new/include -I$(TOP)/zlib -I$(TOP)/iptables-new/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export CPPFLAGS="$(COPTS) -fcommon $(MIPS16_OPT) -fPIC -DNEED_PRINTF  -D_GNU_SOURCE -DHAVE_MALLOC=1 -Drpl_malloc=malloc -I$(TOP)/iptables-new/include -I$(TOP)/zlib -I$(TOP)/iptables-new/include/libipq/ -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink/include -I$(TOP)/libnet/include" ; \
	export LDFLAGS="-L$(TOP)/iptables-new/libipq/.libs -L$(TOP)/libnetfilter_queue/src/.libs $(TOP)/libnetfilter_queue/src/.libs/libnetfilter_queue.a -L$(TOP)/libnet/lib -L$(TOP)/zlib -L$(TOP)/libnfnetlink/src/.libs $(TOP)/libnfnetlink/src/.libs/libnfnetlink.so  -L$(TOP)/libdnet/src/.libs -ldnet -lipq -lnet -L$(TOP)/libpcap -lpcap" ;\
	cd snort && ./configure \
		--enable-reload \
		--enable-ipv6 \
		--libdir=/usr/lib \
		--prefix=/usr \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--without-mysql \
		--without-postgresql \
		--disable-open-appid \
		--enable-pthread \
		--enable-gre \
		--enable-dynamicplugin \
		--enable-react \
		--enable-active-response \
		--enable-flexresp3 \
		--enable-fast-install=yes \
		--with-libpcre-includes="$(TOP)/pcre" \
		--with-libpcre_libraries="$(TOP)/pcre/.libs" \
		--with-libpcap-includes="$(TOP)/libpcap" \
		--with-libpcap-libraries="$(TOP)/libpcap" \
		--with-dnet-includes="$(TOP)/libdnet/include" \
		--with-dnet-libraries="$(TOP)/libdnet/src/.libs" \
		--with-daq-includes="$(TOP)/daq/install/include" \
		--with-daq-libraries="$(TOP)/daq/install/lib" \
		--with-lzma-includes="$(TOP)/xz/src/liblzma/api" \
		--with-lzma-libraries="$(TOP)/xz/src/liblzma/.libs" \
		PATH=$(TOP)/daq/install/bin:$(PATH)
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/src/output-plugins/Makefile
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/src/Makefile
	sed -i 's/\/usr\/include\/pcap/ /g' $(TOP)/snort/tools/u2boat/Makefile
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/snort/ltmain.sh
	sed -i 's/need_relink=yes/need_relink=no/g' $(TOP)/snort/libtool
endif

snort: libnet pcre
	make -C snort CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/librpc"
	make -C snort/so_rules/src CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/librpc -I$(TOP)/daq/install/include -I$(TOP)/pcre -I$(TOP)/zlib"

snort-clean:
	$(MAKE) -C snort clean CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/librpc"

snort-install:
	$(MAKE) -C snort install DESTDIR=$(INSTALLDIR)/snort CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/librpc"
	mkdir -p $(INSTALLDIR)/snort/etc/snort/so_rules
	mkdir -p $(INSTALLDIR)/snort/etc/snort/rules
	cp snort/so_rules/src/*.so $(INSTALLDIR)/snort/etc/snort/so_rules
	cp snort/so_rules/*.rules $(INSTALLDIR)/snort/etc/snort/so_rules
	cp snort/rules/*.rules $(INSTALLDIR)/snort/etc/snort/rules
	cp snort/etc/* $(INSTALLDIR)/snort/etc/snort/
	rm -f $(INSTALLDIR)/snort/etc/snort/Makefile*
	rm -rf $(INSTALLDIR)/snort/usr/src
	rm -rf $(INSTALLDIR)/snort/usr/share
	rm -rf $(INSTALLDIR)/snort/usr/include
	rm -rf $(INSTALLDIR)/snort/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/snort/usr/lib/snort
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicengine/*.la
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicengine/*.a
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicpreprocessor/*.la
	rm -f $(INSTALLDIR)/snort/usr/lib/snort_dynamicpreprocessor/*.a
	rm -rf $(INSTALLDIR)/snort/tmp
