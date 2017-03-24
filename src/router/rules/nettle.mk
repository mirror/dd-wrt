NETTLE_CONFIGURE_ARGS+= \
	--disable-documentation \
	--with-include-path="$(TOP)/openssl/include" \
	--with-lib-path="$(TOP)/openssl , $(TOP)/gmp"

nettle-configure: openssl gmp
	cd nettle && ./configure --host=$(ARCH)-linux --prefix=/usr CC="ccache $(CC)" $(NETTLE_CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/pcre -I$(TOP)/gmp -I$(TOP)/zlib" CPPFLAGS="$(COPTS) $(MIPS16_OPT)" LDFLAGS="-L$(TOP)/pcre/.libs -L$(TOP)/gmp/.libs -lpthread -lpcre -L$(TOP)/zlib $(LDFLAGS) -lz"

nettle: openssl gmp
	make -C nettle 

nettle-clean:
	-make -C nettle clean

nettle-install:
	mkdir -p $(INSTALLDIR)/nettle/usr/lib
ifeq ($(CONFIG_DNSSEC),y)
	install -D nettle/.lib/* $(INSTALLDIR)/nettle/usr/lib
endif
