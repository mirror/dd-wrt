glib20-configure:
	cd glib20/libiconv && ./configure --enable-shared --disable-static --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/libiconv clean all

	cd glib20/gettext && ./configure --enable-shared --disable-static --disable-openmp --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/gettext clean all

	cd glib20/libglib && ./configure --enable-shared --disable-static --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -I$(TOP)/zlib -DNEED_PRINTF -fPIC -Drpl_malloc=malloc -I$(TOP)/glib20/gettext/gettext-runtime/intl  -I$(TOP)/glib20/libiconv/include -L$(TOP)/glib20/libiconv/lib/.libs -L$(TOP)/glib20/gettext/gettext-runtime/intl/.libs -L$(TOP)/zlib" --with-libiconv=gnu
	$(MAKE) -C glib20/libglib clean all

glib20:
	$(MAKE) -C glib20/libiconv all
	$(MAKE) -C glib20/gettext all
	$(MAKE) -C glib20/libglib all

glib20-clean:
	$(MAKE) -C glib20/libiconv clean
	$(MAKE) -C glib20/gettext clean
	$(MAKE) -C glib20/libglib clean

glib20-install:
	install -D glib20/libglib/glib/.libs/libglib-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libglib-2.0.so.0
ifeq ($(CONFIG_MC),y)
	install -D glib20/libglib/gmodule/.libs/libgmodule-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgmodule-2.0.so.0
endif
	install -D glib20/libiconv/lib/.libs/libiconv.so.2 $(INSTALLDIR)/glib20/usr/lib/libiconv.so.2
	install -D glib20/gettext/gettext-runtime/intl/.libs/libintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libintl.so.8
#	install -D glib20/gettext/gettext-runtime/libasprintf/.libs/libasprintf.so.0 $(INSTALLDIR)/glib20/usr/lib/libasprintf.so.0
	install -D glib20/gettext/gettext-runtime/src/.libs/envsubst $(INSTALLDIR)/glib20/usr/bin/envsubst
	install -D glib20/gettext/gettext-runtime/src/.libs/gettext $(INSTALLDIR)/glib20/usr/bin/gettext
	install -D glib20/gettext/gettext-runtime/src/.libs/ngettext $(INSTALLDIR)/glib20/usr/bin/ngettext

