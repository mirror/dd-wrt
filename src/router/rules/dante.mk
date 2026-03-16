dante-configure: libtirpc
	cd dante && rm -f config.cache
	cd dante && libtoolize
	cd dante && aclocal
	cd dante && autoconf
	cd dante && autoheader
	cd dante && autoreconf -vfi
	cd dante && ./configure --host=$(ARCH)-linux --prefix=/usr  --disable-debug --enable-release --without-upnp --without-pam --disable-libwrap CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/libtirpc/tirpc" LDFLAGS="-L$(TOP)/libtirpc/src/.libs -ltirpc"

dante: libtirpc
	make -C dante

dante-install:
	make -C dante install DESTDIR=$(INSTALLDIR)/dante
	rm -rf $(INSTALLDIR)/dante/usr/include
	rm -rf $(INSTALLDIR)/dante/usr/share
	rm -f $(INSTALLDIR)/dante/usr/lib/*.la
	rm -f $(INSTALLDIR)/dante/usr/lib/*.a

dante-clean:
	make -C dante clean

