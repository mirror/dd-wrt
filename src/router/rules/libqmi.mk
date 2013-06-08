libqmi-configure:
	cd libqmi && ./configure --enable-more-warnings=no --enable-static --disable-shared --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/gobject -liconv -lintl -lpthread -DMESSAGE_ENABLE_TRACE" \
	LDFLAGS="-L$(TOP)/glib20/gettext/gettext-runtime/intl/.libs -L$(TOP)/glib20/libiconv/lib/.libs -L$(TOP)/glib20/libglib/gobject/.libs -L$(TOP)/glib20/libglib/gthread/.libs -L$(TOP)/glib20/libglib/gio/.libs" \
	LIBQMI_GLIB_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	LIBQMI_GLIB_LIBS="-pthread -lpthread -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs -lgio-2.0 -lgobject-2.0 -lglib-2.0" \
	QMICLI_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	QMICLI_LIBS="-pthread -lpthread -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gio/.libs -L$(TOP)/glib20/libglib/gobject/.libs  -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs -lgio-2.0 -lgobject-2.0 -lglib-2.0"

libqmi:
	$(MAKE) -C libqmi

libqmi-clean:
	if test -e "libqmi/Makefile"; then make -C libqmi clean; fi
	@true

libqmi-install:
	install -D libqmi/utils/qmi-network $(INSTALLDIR)/libqmi/usr/sbin/qmi-network
	install -D libqmi/cli/qmicli $(INSTALLDIR)/libqmi/usr/sbin/qmicli
#	install -D libqmi/libqmi-glib/.libs/libqmi-glib.so.0.0.0 $(INSTALLDIR)/libqmi/usr/lib/libqmi-glib.so.0
