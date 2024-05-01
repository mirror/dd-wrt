UCFLAGS_DBUS:=-I$(TOP)/dbus/dbus
ULDFLAGS_DBUS:=-L$(TOP)/dbus/dbus/.libs -ldbus-1


avahi-configure: expat-configure expat dbus-configure dbus libdaemon-configure libdaemon
	# runstatedir does not work it defaults to /run, patch configure.ac 1007: avahi_runtime_dir="${localstatedir}/run"
	mkdir -p avahi/build_utils
	mkdir -p avahi/build_normal
	cd avahi && ./autogen.sh NOCONFIGURE=1
	cd avahi/build_utils && ../configure --prefix=/usr --host=$(ARCH)-linux \
		--sysconfdir=/tmp \
		--localstatedir=/tmp/var \
		--with-distro=none \
		--enable-introspection=no \
		--enable-dbus \
		--disable-nls --disable-glib --disable-libevent --disable-gobject \
		--disable-qt3 --disable-qt4 --disable-qt5 --disable-gtk --disable-gtk3 \
		--disable-gdbm --disable-python --disable-python-dbus \
		--disable-mono --disable-monodoc --disable-autoipd \
		--disable-doxygen-doc --disable-manpages --disable-xmltoman \
		--with-xml=expat \
		--with-avahi-user="nobody" --with-avahi-group="nobody" \
		--disable-stack-protector \
		--disable-dependency-tracking \
		CC="$(CC)" \
		LIBDAEMON_CFLAGS="-I$(TOP)/libdaemon" \
		LIBDAEMON_LIBS="-L$(TOP)/libdaemon/libdaemon/.libs -ldaemon" \
		LIBEVENT_CFLAGS="-I$(TOP)/libevent -I$(TOP)/libevent/include" \
		LIBEVENT_LIBS="-L$(TOP)/libevent/.libs" \
		GLIB20_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib" \
		GLIB20_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build/glib" \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -I$(TOP)/expat/lib $(UCFLAGS_DBUS) -Wl,--gc-sections -Drpl_malloc=malloc" \
		CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -L$(TOP)/expat/dynamic/lib/.libs $(ULDFLAGS_DBUS) -ldl -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" 

	cd avahi/build_normal && ../configure --prefix=/usr --host=$(ARCH)-linux \
		--sysconfdir=/tmp \
		--localstatedir=/tmp/var \
		--with-distro=none \
		--enable-introspection=no \
		--disable-dbus \
		--disable-shared \
		--enable-static \
		--disable-nls --disable-glib --disable-libevent --disable-gobject \
		--disable-qt3 --disable-qt4 --disable-qt5 --disable-gtk --disable-gtk3 \
		--disable-gdbm --disable-python --disable-python-dbus \
		--disable-mono --disable-monodoc --disable-autoipd \
		--disable-doxygen-doc --disable-manpages --disable-xmltoman \
		--with-xml=expat \
		--with-avahi-user="nobody" --with-avahi-group="nobody" \
		--disable-stack-protector \
		--disable-dependency-tracking \
		CC="$(CC)" \
		LIBDAEMON_CFLAGS="-I$(TOP)/libdaemon" \
		LIBDAEMON_LIBS="-L$(TOP)/libdaemon/libdaemon/.libs -ldaemon" \
		LIBEVENT_CFLAGS="-I$(TOP)/libevent -I$(TOP)/libevent/include" \
		LIBEVENT_LIBS="-L$(TOP)/libevent/.libs" \
		GLIB20_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib" \
		GLIB20_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build/glib" \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -D_GNU_SOURCE -ffunction-sections -fdata-sections -I$(TOP)/expat/lib -Wl,--gc-sections -Drpl_malloc=malloc" \
		CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNDEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -L$(TOP)/expat/static/lib/.libs -ldl -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" 


avahi:
#	install -D avahi/config/avahi.webservices httpd/ej_temp/avahi.webservices
	-$(MAKE) -C avahi/build_utils
	$(MAKE) -C avahi/build_normal

avahi-clean:
	$(MAKE) -C avahi/build_normal clean
	$(MAKE) -C avahi/build_utils clean

avahi-install:
ifeq ($(CONFIG_MDNS_UTILS),y)
	install -D avahi/build_utils/avahi-daemon/.libs/avahi-daemon $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	install -D avahi/build_utils/avahi-common/.libs/libavahi-common.so.3.5.4 $(INSTALLDIR)/avahi/usr/lib/libavahi-common.so.3.5.4
	install -D avahi/build_utils/avahi-core/.libs/libavahi-core.so.7.1.0 $(INSTALLDIR)/avahi/usr/lib/libavahi-core.so.7.1.0
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-common.so.3.5.4 libavahi-common.so.3
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-core.so.7.1.0 libavahi-core.so.7
	-install -D avahi/config/avahi.nvramconfig $(INSTALLDIR)/avahi/etc/config/zavahi.nvramconfig
#	-install -D avahi/config/avahi.webservices $(INSTALLDIR)/avahi/etc/config/zavahi.webservices

	install -D avahi/build_utils/avahi-utils/.libs/avahi* $(INSTALLDIR)/avahi/usr/sbin/
	install -D avahi/build_utils/avahi-client/.libs/libavahi-client.so.3.2.9 $(INSTALLDIR)/avahi/usr/lib/libavahi-client.so.3.2.9
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-client.so.3.2.9 libavahi-client.so.3
	#copy as *z*avahi so that is wiil not show as first on the webpage 9shoe.webservices in alphabetically order
else
	install -D avahi/build_normal/avahi-daemon/avahi-daemon $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	-install -D avahi/config/avahi.nvramconfig $(INSTALLDIR)/avahi/etc/config/zavahi.nvramconfig
#	-install -D avahi/config/avahi.webservices $(INSTALLDIR)/avahi/etc/config/zavahi.webservices

endif

.PHONY: avahi avahi-configure avahi-install avahi-clean