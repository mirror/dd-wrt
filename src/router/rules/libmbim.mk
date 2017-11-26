GLIB_GIO=-lgio-2.0
libmbim-configure:
	cd libmbim && ./configure --enable-more-warnings=no --enable-static --disable-shared --libexecdir=/usr/lib --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/gobject -I$(TOP)/_staging/usr/include -liconv -lintl -lpthread -DMESSAGE_ENABLE_TRACE" \
	LDFLAGS="-L$(TOP)/glib20/gettext/gettext-runtime/intl/.libs -L$(TOP)/glib20/libiconv/lib/.libs -L$(TOP)/glib20/libglib/gobject/.libs -L$(TOP)/glib20/libglib/gthread/.libs -L$(TOP)/glib20/libglib/gio/.libs -L$(TOP)/_staging/usr/lib" \
	LIBMBIM_GLIB_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	LIBMBIM_GLIB_LIBS="-pthread -lpthread -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs $(GLIB_GIO) -lgobject-2.0 -lglib-2.0" \
	MBIMCLI_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -I$(TOP)/_staging/usr/include -pthread" \
	MBIMCLI_LIBS="-pthread -lpthread -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gio/.libs -L$(TOP)/glib20/libglib/gobject/.libs  -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs -L$(TOP)/_staging/usr/lib $(GLIB_GIO) -lgobject-2.0 -lglib-2.0 -ljson-c -lm" \
	MBIMPROXY_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	MBIMPROXY_LIBS="-pthread -lpthread -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gio/.libs -L$(TOP)/glib20/libglib/gobject/.libs  -L$(TOP)/glib20/libglib/glib/.libs -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/gthread/.libs $(GLIB_GIO) -lgobject-2.0 -lglib-2.0"

libmbim:
	$(MAKE) -C libmbim

libmbim-clean:
	if test -e "libmbim/Makefile"; then make -C libmbim clean; fi
	@true

libmbim-install:
	install -D libmbim/src/mbim-proxy/mbim-proxy $(INSTALLDIR)/libmbim/usr/lib/mbim-proxy
	install -D libmbim/src/mbimcli/mbimcli $(INSTALLDIR)/libmbim/usr/sbin/mbimcli
	install -D libmbim/utils/mbim-network $(INSTALLDIR)/libmbim/usr/sbin/mbim-network
	install -D private/lte-scripte/mbim-connect.sh $(INSTALLDIR)/libmbim/usr/sbin/mbim-connect.sh
	install -D private/lte-scripte/json_to_grepable.awk $(INSTALLDIR)/libmbim/usr/sbin/json_to_grepable.awk
	install -D private/lte-scripte/check-mbim-con-status.sh $(INSTALLDIR)/libmbim/usr/sbin/check-mbim-con-status.sh
	cp private/lte-scripte/mbim-status* $(INSTALLDIR)/libmbim/usr/sbin/
	cp private/lte-scripte/7*xx-set-mode-* $(INSTALLDIR)/libmbim/usr/sbin/
	cp private/lte-scripte/check_mbim_qmi_mode.sh $(INSTALLDIR)/libmbim/usr/sbin/
