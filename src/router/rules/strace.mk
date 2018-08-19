strace-configure:
	cd strace && ./configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -DNEED_PRINTF" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl"

strace: 
	$(MAKE) -C strace

strace-clean: 
	if test -e "strace/Makefile"; then $(MAKE) -C strace clean ; fi

strace-install: 
	$(MAKE) -C strace install DESTDIR=$(INSTALLDIR)/strace
	rm -rf $(INSTALLDIR)/strace/usr/man
	