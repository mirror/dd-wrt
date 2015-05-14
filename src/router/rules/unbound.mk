unbound-configure:
	cd unbound && ./configure --disable-ecdsa \
		--disable-gost \
		--enable-allsymbols \
		--with-ssl="$(TOP)/openssl" \
		--without-pthreads \
		--prefix=/usr \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl"

unbound: 
	$(MAKE) -C unbound

unbound-clean: 
	if test -e "unbound/Makefile"; then $(MAKE) -C unbound clean ; fi

unbound-install: 
	$(MAKE) -C unbound install DESTDIR=$(INSTALLDIR)/unbound