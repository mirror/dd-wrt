dns_responder-configure:
	@true

dns_responder:
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/intatstart/tools clean
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/intatstart/tools

dns_responder-clean:
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS)" -C private/buffalo/intatstart/tools clean

dns_responder-install:
	install -D private/buffalo/intatstart/tools/dns_responder $(INSTALLDIR)/dns_responder/usr/sbin/dns_responder
