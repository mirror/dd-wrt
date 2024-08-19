krb5-configure:
#	cd krb5 && ./autogen.sh
	cd krb5/src && ./configure --libdir=/usr/lib --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(THUMB) -DNEED_PRINTF -D_GNU_SOURCE -I$(SSLPATH)/include" \
		LDFLAGS="$(COPTS) -L$(SSLPATH)" \
		--prefix=/usr \
		--without-system-verto \
		krb5_cv_attr_constructor_destructor=yes \
		ac_cv_func_regcomp=yes \
		ac_cv_printf_positional=no

krb5: libtirpc
	make -C krb5/src

krb5-clean:
	make -C krb5/src clean

krb5-install:
	make -C krb5/src install DESTDIR=$(INSTALLDIR)/krb5
	find $(INSTALLDIR)/krb5 -name *.la -delete
	rm -rf $(INSTALLDIR)/krb5/usr/include
	rm -rf $(INSTALLDIR)/krb5/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/krb5/usr/share
	rm -rf $(INSTALLDIR)/krb5/var
	rm -rf $(INSTALLDIR)/krb5/run
	rm -f $(INSTALLDIR)/krb5/usr/lib/*.a
	rm -f $(INSTALLDIR)/krb5/usr/lib/*.la
