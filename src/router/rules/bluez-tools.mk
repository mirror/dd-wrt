
bluez-tools-configure:
	cd bluez-tools && autoreconf --force --install --verbose
	rm -f bluez-tools/config.cache
	cd bluez-tools && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
	--sysconfdir=/etc \
	--localstatedir=/tmp/var \
	GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build -I$(TOP)/glib20/libglib/build/glib" \
	GLIB_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib -L$(TOP)/glib20/libglib/build/gthread -L$(TOP)/glib20/libglib/build/gio -L$(TOP)/glib20/libglib/build/gobject" \
	GIO_CFLAGS="-I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build -I$(TOP)/glib20/libglib/build/glib -I$(TOP)/glib20/libglib/build/gio -I$(TOP)/glib20/libglib/build/gmodule -I$(TOP)/glib20/libglib/gmodule" \
	GIO_LIBS="-L$(TOP)/glib20/libglib/build/gio -lgio-2.0 -L$(TOP)/glib20/libglib/build/glib -lglib-2.0 -L$(TOP)/glib20/libglib/build/gobject -lgobject-2.0" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -I$(TOP) -I$(TOP)/zlib  -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -UHAVE_SELINUX -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
	LDFLAGS="$(LDLTO) -L$(TOP)/dbus/dbus/.libs -L$(TOP)/readline/shlib -L$(TOP)/ncurses/lib -lncurses -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi  -L$(TOP)/zlib -lz -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	ac_cv_lib_rt_clock_gettime=yes

bluez-tools:
	$(MAKE) -C bluez-tools

bluez-tools-install:
	$(MAKE) -C bluez-tools DESTDIR=$(INSTALLDIR)/bluez-tools install
	rm -rf $(INSTALLDIR)/bluez-tools/usr/share

bluez-tools-clean:
	-$(MAKE) -C bluez-tools clean

.PHONY: bluez-tools-configure bluez-tools bluez-tools-install bluez-tools-clean
