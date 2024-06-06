libqmi-configure:
	cd libqmi && ./configure --enable-more-warnings=no --enable-static --disable-shared --host=$(ARCH)-linux CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/gobject -lpthread -DMESSAGE_ENABLE_TRACE" \
	LDFLAGS="-L$(TOP)/glib20/gettext/gettext-runtime/intl/.libs -L$(TOP)/glib20/libiconv/lib/.libs -L$(TOP)/glib20/libglib/build/gobject -L$(TOP)/glib20/libglib/build/gthread -L$(TOP)/glib20/libglib/build/gio" \
	LIBQMI_GLIB_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc  -Wno-incompatible-pointer-types -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/build -I$(TOP)/glib20/libglib/build/glib -I$(TOP)/glib20/libglib/build/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	LIBQMI_GLIB_LIBS="-pthread -lpthread -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi  -L$(TOP)/zlib -lz -L$(TOP)/glib20/libglib/build/glib -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs -lgio-2.0 -lgobject-2.0 -lglib-2.0" \
	QMICLI_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc  -Wno-incompatible-pointer-types -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/build -I$(TOP)/glib20/libglib/build/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	QMICLI_LIBS="-pthread -lpthread -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/build/gio -L$(TOP)/glib20/libglib/build/gobject  -L$(TOP)/glib20/libglib/build/glib -L$(TOP)/glib20/libglib/build/gmodule -L$(TOP)/glib20/libglib/build/gthread -lgio-2.0 -lgobject-2.0 -lglib-2.0"
	QMICLI_LIBS=" -ffunction-sections -fdata-sections -Wl,--gc-sections"

libqmi:
	$(MAKE) -C libqmi

libqmi-clean:
	if test -e "libqmi/Makefile"; then make -C libqmi clean; fi
	@true

libqmi-install:
	install -D libqmi/utils/qmi-network $(INSTALLDIR)/libqmi/usr/sbin/qmi-network
	install -D libqmi/cli/qmicli $(INSTALLDIR)/libqmi/usr/sbin/qmicli
#	install -D libqmi/libqmi-glib/.libs/libqmi-glib.so.0.0.0 $(INSTALLDIR)/libqmi/usr/lib/libqmi-glib.so.0
