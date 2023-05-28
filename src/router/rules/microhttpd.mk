
libmicrohttpd-configure:
	cd libmicrohttpd && libtoolize
	cd libmicrohttpd && aclocal
	cd libmicrohttpd && autoconf
	cd libmicrohttpd && autoheader
	cd libmicrohttpd && autoreconf -vfi
	cd libmicrohttpd && ./configure  --host=$(ARCH)-linux  --disable-messages --without-openssl --without-gnutls --disable-spdy --disable-https --prefix=/usr --libdir=/usr/lib --disable-shared --enable-static \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections" LDFLAGS="$(LDLTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

libmicrohttpd:
	$(MAKE) -C libmicrohttpd

libmicrohttpd-install:
	@true
