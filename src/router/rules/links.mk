links-configure:
	export CFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" CPPFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" ; \
	export CXXFLAGS="$(COPTS) -DNEED_PRINTF -L$(TOP)/openssl" ; \
	cd links && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr --with-ssl=$(TOP)/openssl

links:
	make -C links

links-clean:
	if test -e "links/Makefile"; then make -C links clean; fi
	@true

links-install:
	install -D links/links $(INSTALLDIR)/links/usr/bin/links
