NETTLE_CONFIGURE_ARGS+= \
	--disable-documentation \
	--with-include-path="$(TOP)/openssl/include" \
	--with-lib-path="$(TOP)/openssl , $(TOP)/gmp"

nettle-configure: pcre zlib openssl gmp
	cd nettle && ./configure --host=$(ARCH)-linux --disable-pic --prefix=/usr CC="ccache $(CC)" $(NETTLE_CONFIGURE_ARGS) CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -I$(TOP)/pcre -I$(TOP)/gmp -I$(TOP)/zlib" CPPFLAGS="$(COPTS) $(MIPS16_OPT)" LDFLAGS="-L$(TOP)/pcre/.libs -L$(TOP)/gmp/.libs -lpthread -lpcre -L$(TOP)/zlib $(LDFLAGS) -lz"

nettle: openssl gmp
	make -C nettle 

nettle-clean:
	-make -C nettle clean

nettle-install:
#	make -C nettle install DESTDIR=$(INSTALLDIR)/nettle
#	-mv $(INSTALLDIR)/nettle/usr/lib64 $(INSTALLDIR)/nettle/usr/lib
#	rm -rf $(INSTALLDIR)/nettle/usr/lib/pkgconfig
#	rm -rf $(INSTALLDIR)/nettle/usr/include
#	rm -rf $(INSTALLDIR)/nettle/usr/bin
#	rm -f $(INSTALLDIR)/nettle/usr/lib/*.a
