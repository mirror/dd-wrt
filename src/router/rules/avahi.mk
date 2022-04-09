
ifeq ($(CONFIG_MDNS_UTILS),y)
	AVAHI_BDBUS:=--enable-dbus
	CFLAGS_DBUS:=-I$(TOP)/dbus/dbus
	LDFLAGS_DBUS:=-L$(TOP)/dbus/dbus/.libs -ldbus-1
else
	AVAHI_BDBUS:=--disable-dbus
endif


avahi-configure: expat dbus libdaemon
	#cd avahi && ./autogen.sh
	#cd avahi && ./configure libmnl_CFLAGS="-I$(TOP)/libmnl/include" -I$(TOP)/libevent -I$(TOP)/libevent/include --prefix=/usr --host=$(ARCH)-linux \\
	#cd avahi && ./configure --prefix=/usr --host=$(ARCH)-linux \\
	# runstatedir does not work it defaults to /run, patch configure.ac 1007: avahi_runtime_dir="${localstatedir}/run"
	cd avahi && ./autogen.sh --prefix=/usr --host=$(ARCH)-linux \
		--sysconfdir=/tmp \
		--localstatedir=/tmp/var \
		--with-distro=none \
		--enable-introspection=no \
		$(AVAHI_BDBUS) \
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
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -I$(TOP)/expat/lib $(CFLAGS_DBUS) -Wl,--gc-sections -Drpl_malloc=malloc" \
		CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -L$(TOP)/expat/lib/.libs $(LDFLAGS_DBUS) -ldl -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" 


avahi:
	install -D avahi/config/avahi.webservices httpd/ej_temp/avahi.webservices
	$(MAKE) -C avahi

ifeq ($(CONFIG_MDNS_UTILS),y)
avahi-clean: expat-clean dbus-clean libdaemon-clean
else
avahi-clean: expat-clean libdaemon-clean
endif
	$(MAKE) -C avahi clean
	rm -f avahi/stamp-h1

ifeq ($(CONFIG_MDNS_UTILS),y)
avahi-install: expat-install dbus-install libdaemon-install
	install -D avahi/avahi-daemon/.libs/avahi-daemon $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	install -D avahi/avahi-common/.libs/libavahi-common.so.3.5.4 $(INSTALLDIR)/avahi/usr/lib/libavahi-common.so.3.5.4
	install -D avahi/avahi-core/.libs/libavahi-core.so.7.1.0 $(INSTALLDIR)/avahi/usr/lib/libavahi-core.so.7.1.0
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-common.so.3.5.4 libavahi-common.so.3
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-core.so.7.1.0 libavahi-core.so.7
	install -D avahi/avahi-utils/.libs/avahi* $(INSTALLDIR)/avahi/usr/sbin/
	install -D avahi/avahi-client/.libs/libavahi-client.so.3.2.9 $(INSTALLDIR)/avahi/usr/lib/libavahi-client.so.3.2.9
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-client.so.3.2.9 libavahi-client.so.3
	#copy as *z*avahi so that is wiil not show as first on the webpage 9shoe.webservices in alphabetically order
	-install -D avahi/config/avahi.nvramconfig $(INSTALLDIR)/avahi/etc/config/zavahi.nvramconfig
	-install -D avahi/config/avahi.webservices $(INSTALLDIR)/avahi/etc/config/zavahi.webservices
else
avahi-install: expat-install libdaemon-install
endif
	install -D avahi/avahi-daemon/.libs/avahi-daemon $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	install -D avahi/avahi-common/.libs/libavahi-common.so.3.5.4 $(INSTALLDIR)/avahi/usr/lib/libavahi-common.so.3.5.4
	install -D avahi/avahi-core/.libs/libavahi-core.so.7.1.0 $(INSTALLDIR)/avahi/usr/lib/libavahi-core.so.7.1.0
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-common.so.3.5.4 libavahi-common.so.3
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-core.so.7.1.0 libavahi-core.so.7
	-install -D avahi/config/avahi.nvramconfig $(INSTALLDIR)/avahi/etc/config/zavahi.nvramconfig
	-install -D avahi/config/avahi.webservices $(INSTALLDIR)/avahi/etc/config/zavahi.webservices

.PHONY: avahi avahi-configure avahi-install avahi-clean