links-configure:
	export CFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" CPPFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" ; \
	export CXXFLAGS="$(COPTS) -DNEED_PRINTF -L$(SSLPATH)" ; \
	cd links && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr --with-ssl=$(SSLPATH)

links:
	make -C links

links-clean:
	if test -e "links/Makefile"; then make -C links clean; fi
	@true

links-install:
	install -D links/links $(INSTALLDIR)/links/usr/bin/links
