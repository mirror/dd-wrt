nrpe-configure:
	cd nrpe && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" CPPFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" CXXFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" \
	--with-ssl=$(SSLPATH) \
	--with-nrpe-user=root \
	--with-nrpe-group=root \
	--with-nagios-user=root \
	--with-nagios-group=root



nrpe:
	cd nrpe && openssl dhparam -C 512 | awk '/^-----/ {exit} {print}' > include/dh.h
	make -C nrpe

nrpe-clean:
	if test -e "nrpe/Makefile"; then make -C nrpe clean; fi
	@true

nrpe-install:
	install -D nrpe/src/nrpe $(INSTALLDIR)/nrpe/usr/bin/nrpe


