ifeq ($(ARCH),arm)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),mips64)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),i386)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),x86_64)
UNBOUND_COPTS += -DNEED_PRINTF
endif
ifeq ($(ARCH),aarch64)
UNBOUND_COPTS += -DNEED_PRINTF
endif

unbound-configure:
	cd unbound && ./configure --disable-ecdsa \
		--disable-gost \
		--enable-allsymbols \
		--enable-tfo-client \
		--enable-tfo-server \
		--enable-subnet \
		--with-chroot-dir=/tmp \
		--with-ssl="$(SSLPATH)" \
		--with-pthreads \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(UNBOUND_COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH)"

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
	rm -rf $(INSTALLDIR)/unbound/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/unbound/usr/lib/*.la
#	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-checkconf
#	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-control-setup
	rm -f $(INSTALLDIR)/unbound/usr/sbin/unbound-host
	