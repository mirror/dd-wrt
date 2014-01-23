squid-configure:
	cd squid && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl -pthread" CPPFLAGS="$(COPTS) -DNEED_PRINTF -pthread -L$(TOP)/openssl" CXXFLAGS="$(COPTS) -DNEED_PRINTF -pthread -L$(TOP)/openssl" \
	ac_cv_header_linux_netfilter_ipv4_h=yes \
	ac_cv_epoll_works=yes \
	--datadir=/usr/local/squid \
	--libexecdir=/usr/lib/squid \
	--sysconfdir=/etc/squid \
	--enable-shared \
	--enable-static \
	--enable-x-accelerator-vary \
	--with-pthreads \
	--with-dl \
	--enable-icmp \
	--enable-kill-parent-hack \
	--enable-arp-acl \
	--enable-ssl \
	--enable-htcp \
	--enable-err-languages=English \
	--enable-default-err-language=English \
	--enable-linux-netfilter \
	--enable-icmp \
	--disable-wccp \
	--disable-wccpv2 \
	--disable-snmp \
	--disable-htcp \
	--enable-underscores \
	--enable-cache-digests \
	--enable-referer-log \
	--enable-delay-pools \
	--enable-useragent-log \
	--with-openssl=$(TOP)/openssl \
	--disable-external-acl-helpers \
	--disable-auth-negotiate \
	--disable-auth-ntlm \
	--disable-auth-digest \
	--disable-auth-basic \
	--enable-epoll \
	--with-krb5-config=no \
	--with-maxfd=4096
	
squid:
	make -C squid
	make -C squid/plugins/squid_radius_auth 

squid-install:
	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	
	rm -rf $(INSTALLDIR)/squid/usr/share
	rm -rf $(INSTALLDIR)/squid/usr/include
	make -C squid/plugins/squid_radius_auth install DESTDIR=$(INSTALLDIR)/squid

