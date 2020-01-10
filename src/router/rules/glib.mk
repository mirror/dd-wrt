ifeq ($(ARCH),i386)
	export SUBARCH:=pc
else
ifeq ($(ARCH),x86_64)
	export SUBARCH:=pc
else
	export SUBARCH:=unknown
endif
endif


glib20-configure: libffi zlib util-linux
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la

	cd glib20/gettext && ./autogen.sh
	cd glib20/gettext && ./configure --enable-static --disable-shared --host=$(ARCH)-linux  LDFLAGS="$(COPTS) $(LTO) -std=gnu89 $(MIPS16_OPT) -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc " CFLAGS="$(COPTS)  $(MIPS16_OPT)  -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc" CXXFLAGS="$(COPTS)  $(MIPS16_OPT) -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc"
	$(MAKE) -C glib20/gettext clean all

	-cd glib20/libglib && ./autogen.sh
	cd glib20/libglib && ./configure --enable-shared --enable-static --disable-fam --with-libiconv=gnu --disable-libelf --with-pcre=internal --disable-libmount --enable-debug=no --disable-selinux --disable-man --host=$(ARCH)-linux  CFLAGS="$(COPTS) $(LTO) -std=gnu89  -DNVALGRIND=1 $(MIPS16_OPT) -D_GNU_SOURCE=1  -I$(TOP)/zlib -fPIC -Drpl_malloc=malloc -I$(TOP)/glib20/gettext -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include  -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/glib20/gettext/.libs -L$(TOP)/glib20/libglib/gmodule/.libs   -L$(TOP)/zlib -L$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/lib -pthread -lpthread -lz" --disable-modular-tests \
	LIBFFI_CFLAGS="-I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include" \
	LIBFFI_LIBS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi" \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	glib_cv_stack_grows=no glib_cv_uscore=no ac_cv_func_mmap_fixed_mapped=yes ac_cv_func_posix_getpwuid_r=yes ac_cv_func_posix_getgrgid_r=yes

	$(MAKE) -C glib20/libglib clean all

glib20: libffi zlib util-linux util-linux-install
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	$(MAKE) -C glib20/gettext all
	$(MAKE) -C glib20/libglib all
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_ZFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*



glib20-clean:
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
ifeq ($(CONFIG_LIBMBIM),y)
	install -D glib20/libglib/gmodule/.libs/libgmodule-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgmodule-2.0.so.0
	install -D glib20/libglib/gthread/.libs/libgthread-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgthread-2.0.so.0
	install -D glib20/libglib/gobject/.libs/libgobject-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgobject-2.0.so.0
	install -D glib20/libglib/gio/.libs/libgio-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libgio-2.0.so.0
endif
	-install -D glib20/gettext/gettext-runtime/intl/.libs/libintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libintl.so.8
	-install -D glib20/gettext/gettext-runtime/intl/.libs/libgnuintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libgnuintl.so.8
#	install -D glib20/gettext/gettext-runtime/libasprintf/.libs/libasprintf.so.0 $(INSTALLDIR)/glib20/usr/lib/libasprintf.so.0
	-install -D glib20/gettext/gettext-runtime/src/.libs/envsubst $(INSTALLDIR)/glib20/usr/bin/envsubst
	-install -D glib20/gettext/gettext-runtime/src/.libs/gettext $(INSTALLDIR)/glib20/usr/bin/gettext
	-install -D glib20/gettext/gettext-runtime/src/.libs/ngettext $(INSTALLDIR)/glib20/usr/bin/ngettext

