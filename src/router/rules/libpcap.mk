libpcap-configure:
	cd libpcap ; ac_cv_linux_vers=2 ./configure \
		--target=$(ARCH)-openwrt-linux \
		--host=$(ARCH)-linux-uclibc  \
		--program-prefix= --program-suffix= --prefix=/usr \
		--exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin \
		--libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share \
		--localstatedir=/var --mandir=/usr/man --infodir=/usr/info \
		--disable-nls --enable-shared --enable-static \
		--disable-yydebug --enable-ipv6 --with-build-cc=gcc \
		--with-pcap=linux --without-septel --without-dag \
		--disable-canusb --disable-can --disable-bluetooth \
		CC="$(CC)" CFLAGS="$(COPTS) -fPIC"
libpcap:
#	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB)
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
	-cd libpcap ; ln -s libpcap.so.1.9.1 libpcap.so
	-cd libpcap ; ln -s libpcap.so libpcap.so.1
else
	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.a 
endif

libpcap-install:
	@true
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	install -d $(INSTALLDIR)/libpcap/usr/lib
	install libpcap/libpcap.so $(INSTALLDIR)/libpcap/usr/lib
	-cd $(INSTALLDIR)/libpcap/usr/lib ; ln -s libpcap.so libpcap.so.1 
	-cd $(INSTALLDIR)/libpcap/usr/lib ; ln -s libpcap.so libpcap.so.1.0.0
endif

libpcap-clean:
	$(MAKE) -C libpcap clean

