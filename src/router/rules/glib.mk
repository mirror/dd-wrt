ifeq ($(ARCH),i386)
	export SUBARCH:=pc
else
ifeq ($(ARCH),x86_64)
	export SUBARCH:=pc
else
	export SUBARCH:=unknown
endif
endif


glib20-configure:
	cd glib20/libffi && ./configure --enable-static --disable-shared --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/libffi clean all

	cd glib20/libiconv && ./configure --enable-shared --enable-static --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/libiconv clean all

	cd glib20/gettext && ./configure --enable-shared --disable-static --disable-openmp --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS)  $(MIPS16_OPT) -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/gettext clean all

	cd glib20/libglib && ./configure --enable-shared --disable-static --disable-fam  --enable-debug=no --disable-selinux --host=$(ARCH)-linux CC="ccache $(CC)" CFLAGS="$(COPTS)  $(MIPS16_OPT) -I$(TOP)/zlib -fPIC -Drpl_malloc=malloc -I$(TOP)/glib20/gettext/gettext-runtime/intl  -I$(TOP)/glib20/libiconv/include -I$(TOP)/glib20/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include  -L$(TOP)/glib20/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/glib20/libiconv/lib/.libs -L$(TOP)/glib20/gettext/gettext-runtime/intl/.libs -L$(TOP)/zlib -pthread -lpthread" --with-libiconv=gnu --disable-modular-tests \
	LIBFFI_CFLAGS="-I$(TOP)/glib20/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include" \
	LIBFFI_LIBS="-L$(TOP)/glib20/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	glib_cv_stack_grows=no glib_cv_uscore=no ac_cv_func_mmap_fixed_mapped=yes ac_cv_func_posix_getpwuid_r=yes ac_cv_func_posix_getgrgid_r=yes

	$(MAKE) -C glib20/libglib clean all

glib20:
	$(MAKE) -C glib20/libffi all
	$(MAKE) -C glib20/libiconv all
	$(MAKE) -C glib20/gettext all
	$(MAKE) -C glib20/libglib all

glib20-clean:
	$(MAKE) -C glib20/libffi clean
	$(MAKE) -C glib20/libiconv clean
	$(MAKE) -C glib20/gettext clean
	$(MAKE) -C glib20/libglib clean

glib20-install:
	install -D glib20/libglib/glib/.libs/libglib-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libglib-2.0.so.0
ifeq ($(CONFIG_MC),y)
	install -D glib20/libglib/gmodule/.libs/libgmodule-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgmodule-2.0.so.0
endif
ifeq ($(CONFIG_LIBQMI),y)
	install -D glib20/libglib/gmodule/.libs/libgmodule-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgmodule-2.0.so.0
	install -D glib20/libglib/gthread/.libs/libgthread-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgthread-2.0.so.0
	install -D glib20/libglib/gobject/.libs/libgobject-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgobject-2.0.so.0
	install -D glib20/libglib/gio/.libs/libgio-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgio-2.0.so.0
endif
	install -D glib20/libiconv/lib/.libs/libiconv.so.2 $(INSTALLDIR)/glib20/usr/lib/libiconv.so.2
	install -D glib20/gettext/gettext-runtime/intl/.libs/libintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libintl.so.8
#	install -D glib20/gettext/gettext-runtime/libasprintf/.libs/libasprintf.so.0 $(INSTALLDIR)/glib20/usr/lib/libasprintf.so.0
	install -D glib20/gettext/gettext-runtime/src/.libs/envsubst $(INSTALLDIR)/glib20/usr/bin/envsubst
	install -D glib20/gettext/gettext-runtime/src/.libs/gettext $(INSTALLDIR)/glib20/usr/bin/gettext
	install -D glib20/gettext/gettext-runtime/src/.libs/ngettext $(INSTALLDIR)/glib20/usr/bin/ngettext

