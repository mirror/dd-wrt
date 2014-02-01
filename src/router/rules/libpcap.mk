libpcap-configure:
	cd libpcap_noring ; ac_cv_linux_vers=2 ./configure \
		--target=$(ARCH)-openwrt-linux \
		--host=$(ARCH)-linux-uclibc  \
		--program-prefix= --program-suffix= --prefix=/usr \
		--exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin \
		--libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share \
		--localstatedir=/var --mandir=/usr/man --infodir=/usr/info \
		--disable-nls --enable-shared --enable-static \
		--disable-yydebug --enable-ipv6 --with-build-cc=gcc \
		--with-pcap=linux --without-septel --without-dag \
		CC="$(CC)" CFLAGS="$(COPTS) -fPIC"
libpcap:
#	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
	$(MAKE) -C libpcap_noring CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) version.h
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	$(MAKE) -C libpcap_noring CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
	-cd libpcap_noring ; ln -s libpcap.so.1.0.0 libpcap.so
else
	$(MAKE) -C libpcap_noring CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.a 
endif

libpcap-install:
	@true
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	install -d $(INSTALLDIR)/libpcap/usr/lib
	install libpcap_noring/libpcap.so $(INSTALLDIR)/libpcap/usr/lib
	-cd $(INSTALLDIR)/libpcap/usr/lib ; ln -s libpcap.so libpcap.so.1 
	-cd $(INSTALLDIR)/libpcap/usr/lib ; ln -s libpcap.so libpcap.so.1.0.0
endif

libpcap-clean:
	$(MAKE) -C libpcap_noring clean

