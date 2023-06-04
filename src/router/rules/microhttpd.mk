
libmicrohttpd-configure:
	cd libmicrohttpd && libtoolize
	cd libmicrohttpd && aclocal
	cd libmicrohttpd && autoconf
	cd libmicrohttpd && autoheader
	cd libmicrohttpd && autoreconf -vfi
	cd libmicrohttpd && ./configure \
		--host=$(ARCH)-linux \
		--disable-doc \
		--without-gnutls \
		--disable-https \
		--prefix=/usr \
		--libdir=/usr/lib \
		--disable-shared \
		--enable-static \
		--disable-curl \
		--disable-rpath \
		--disable-examples \
		--disable-poll \
		--enable-epoll \
		--disable-messages \
		--with-pic \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections" LDFLAGS="$(LDLTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

libmicrohttpd:
	$(MAKE) -C libmicrohttpd

libmicrohttpd-install:
	@true
