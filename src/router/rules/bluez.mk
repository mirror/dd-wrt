
bluez-configure: dbus
	cd bluez && autoreconf -fsi
	rm -f bluez/config.cache
	cd bluez && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
	--sysconfdir=/etc \
	--localstatedir=/tmp/var \
	--disable-static \
	--enable-shared \
	--enable-client \
	--enable-mesh \
	--enable-btpclient \
	--enable-datafiles \
	--enable-experimental \
	--enable-library \
	--enable-monitor \
	--disable-obex \
	--enable-threads \
	--enable-tools \
	--disable-android \
	--disable-cups \
	--disable-manpages \
	--enable-sixaxis \
	--disable-systemd \
	--disable-test \
	--disable-udev \
	--enable-deprecated \
	DBUS_CFLAGS="-I$(TOP)/dbus/staged/usr/include/dbus-1.0 -I$(TOP)/dbus/staged/usr/lib/dbus-1.0/include" \
	GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build -I$(TOP)/glib20/libglib/build/glib" \
	GLIB_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib -L$(TOP)/glib20/libglib/build/gthread -L$(TOP)/glib20/libglib/build/gio -L$(TOP)/glib20/libglib/build/gobject -lglib-2.0" \
	GTHREAD_CFLAGS="-I$(TOP)/glib20/libglib/glib" \
	GTHREAD_LIBS="-L$(TOP)/glib20/libglib/build/gthread" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -I$(TOP) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -UHAVE_SELINUX -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
	LDFLAGS="$(LDLTO) -L$(TOP)/dbus/dbus/.libs -L$(TOP)/readline/shlib -L$(TOP)/json-c/.libs -L$(TOP)/ncurses/lib -lncurses -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	ac_cv_lib_rt_clock_gettime=yes

bluez:
	$(MAKE) -C bluez

bluez-install:
	$(MAKE) -C bluez DESTDIR=$(INSTALLDIR)/bluez install
	mv $(INSTALLDIR)/bluez/usr/lib/bluetooth/bluetoothd $(INSTALLDIR)/bluez/usr/bin
	mv $(INSTALLDIR)/bluez/usr/lib/bluetooth/bluetooth-meshd $(INSTALLDIR)/bluez/usr/bin
	rm -rf $(INSTALLDIR)/bluez/usr/include
	rm -f $(INSTALLDIR)/bluez/usr/lib/*.la
	rm -f $(INSTALLDIR)/bluez/usr/lib/*.a
	rm -rf $(INSTALLDIR)/bluez/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/bluez/usr/share
	rm -rf $(INSTALLDIR)/bluez/usr/var

bluez-clean:
	-$(MAKE) -C bluez clean

.PHONY: bluez-configure bluez bluez-install bluez-clean
