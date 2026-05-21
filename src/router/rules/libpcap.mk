libpcap-configure:
	rm -f  libpcap.old/libpcap*.so*
	cd libpcap.old ; ac_cv_linux_vers=2; ac_cv_prog_cc_c99=yes; ./configure \
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
		--disable-usb \
		--without-libnl \
		--disable-canusb --disable-can --disable-bluetooth \
		CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -std=c99 -D_GNU_SOURCE -fPIC"
libpcap:
	rm -f $(TOP)/libpcap.old/libpcap.a
	rm -f $(TOP)/libpcap.old/libpcap.so
	$(MAKE) -C libpcap.old CC="$(CC)" AR=$(AR) RANLIB=$(RANLIB) libpcap.so
	cp -f $(TOP)/libpcap.old/libpcap.so* $(TOP)/libpcap/libpcap.so

libpcap-install:
	$(MAKE) -C libpcap.old install DESTDIR=$(INSTALLDIR)/libpcap
	rm -rf $(INSTALLDIR)/libpcap/usr/bin
	rm -rf $(INSTALLDIR)/libpcap/usr/include
	rm -rf $(INSTALLDIR)/libpcap/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libpcap/usr/man
	rm -f $(INSTALLDIR)/libpcap/usr/lib/libpcap.a

libpcap-clean:
	$(MAKE) -C libpcap clean

