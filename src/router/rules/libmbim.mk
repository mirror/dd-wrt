GLIB_GIO=-lgio-2.0 -lgmodule-2.0
libmbim-configure: glib20
	#cd libmbim && ./autogen.sh
	cd libmbim && ./configure --enable-more-warnings=no --enable-static --disable-shared --libexecdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/_staging/usr/lib -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc  -I$(TOP)/_staging/usr/lib/glib-2.0/include -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib/gobject -I$(TOP)/_staging/usr/include -lpthread -DMESSAGE_ENABLE_TRACE" \
	LDFLAGS="-L$(TOP)/glib20/gettext/.libs -L$(TOP)/_staging/usr/lib -lpcre2-8" \
	LIBMBIM_GLIB_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc  -L$(TOP)/_staging/usr/lib -I$(TOP)/_staging/usr/lib/glib-2.0/include -I$(TOP)/_staging/usr/include/glib-2.0 -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	LIBMBIM_GLIB_LIBS="-pthread -lpthread -L$(TOP)/_staging/usr/lib $(GLIB_GIO) -lgobject-2.0 -lglib-2.0 -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz" \
	MBIMCLI_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -I$(TOP)/_staging/usr/lib/glib-2.0/include -I$(TOP)/_staging/usr/include/glib-2.0 -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -I$(TOP)/_staging/usr/include -pthread" \
	MBIMCLI_LIBS="-pthread -lpthread -L$(TOP)/_staging/usr/lib $(GLIB_GIO) -lgobject-2.0 -lglib-2.0 -ljson-c -lm -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz" \
	MBIMPROXY_CFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_realloc=realloc -Drpl_malloc=malloc -L$(TOP)/_staging/usr/lib -I$(TOP)/_staging/usr/lib/glib-2.0/include -I$(TOP)/_staging/usr/include/glib-2.0 -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/gmodule -pthread" \
	MBIMPROXY_LIBS="-pthread -lpthread -L$(TOP)/_staging/usr/lib $(GLIB_GIO) -lgobject-2.0 -lglib-2.0 -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz"

libmbim: glib20
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
	install -D private/lte-scripte/7304-mbim.comgt $(INSTALLDIR)/libmbim/etc/comgt/7304-mbim.comgt
	install -D private/lte-scripte/7304-qmi.comgt $(INSTALLDIR)/libmbim/etc/comgt/7304-qmi.comgt
	cp private/lte-scripte/mbim-status* $(INSTALLDIR)/libmbim/usr/sbin/
	cp private/lte-scripte/7*xx-set-mode-* $(INSTALLDIR)/libmbim/usr/sbin/
	cp private/lte-scripte/check_mbim_qmi_mode.sh $(INSTALLDIR)/libmbim/usr/sbin/
	cp private/lte-scripte/deploy* $(INSTALLDIR)/libmbim/usr/sbin/
