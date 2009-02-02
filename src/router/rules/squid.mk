squid-configure:
	cd squid && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" CPPFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" CXXFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" \
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
	--enable-external-acl-helpers="" \
	--enable-underscores \
	--enable-cache-digests \
	--enable-referer-log \
	--enable-delay-pools \
	--enable-useragent-log \
	--with-openssl=$(TOP)/openssl \
	--enable-auth="basic digest ntlm" \
	--enable-basic-auth-helpers="getpwnam NCSA SMB" \
	--enable-ntlm-auth-helpers="fakeauth SMB" \
	--enable-digest-auth-helpers="password" \
	--enable-external-acl-helpers="ip_user unix_group" \
	--enable-epoll \
	--with-maxfd=4096 \


squid:
	make -C squid

squid-install:
	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	

