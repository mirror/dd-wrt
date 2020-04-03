libpcap-configure:
	cd libpcap ; ac_cv_linux_vers=2 ./configure \
		--target=$(ARCH)-openwrt-linux \
		--host=$(ARCH)-linux-uclibc  \
		--libdir=/usr/lib \
		--program-prefix= --program-suffix= --prefix=/usr \
		--exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin \
		--libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share \
		--localstatedir=/var --mandir=/usr/man --infodir=/usr/info \
		--disable-nls --disable-static \
		--disable-yydebug --enable-ipv6 --with-build-cc=gcc \
		--with-pcap=linux --without-septel --without-dag \
		--disable-canusb --disable-can --disable-bluetooth \
		CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC"
libpcap:
	$(MAKE) -C libpcap CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so 

libpcap-install:
	$(MAKE) -C libpcap install DESTDIR=$(INSTALLDIR)/libpcap
	rm -rf $(INSTALLDIR)/libpcap/usr/bin
	rm -rf $(INSTALLDIR)/libpcap/usr/include
	rm -rf $(INSTALLDIR)/libpcap/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libpcap/usr/man
	rm -f $(INSTALLDIR)/libpcap/usr/lib/libpcap.a

libpcap-clean:
	$(MAKE) -C libpcap clean

