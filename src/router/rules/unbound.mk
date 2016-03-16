unbound-configure:
	cd unbound && ./configure --disable-ecdsa \
		--disable-gost \
		--enable-allsymbols \
		--with-chroot-dir=/tmp \
		--with-ssl="$(TOP)/openssl" \
		--without-pthreads \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl"

unbound: 
	$(MAKE) -C unbound

unbound-clean: 
	if test -e "unbound/Makefile"; then $(MAKE) -C unbound clean ; fi

unbound-install: 
	$(MAKE) -C unbound install DESTDIR=$(INSTALLDIR)/unbound
	mkdir -p $(INSTALLDIR)/unbound/etc/unbound
	cp unbound/config/* $(INSTALLDIR)/unbound/etc/unbound
	rm -rf $(INSTALLDIR)/unbound/usr/include
	rm -rf $(INSTALLDIR)/unbound/usr/share
	rm -f $(INSTALLDIR)/unbound/usr/lib/*.a
	rm -f $(INSTALLDIR)/unbound/usr/lib/*.la
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-checkconf
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control-setup
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-host
	