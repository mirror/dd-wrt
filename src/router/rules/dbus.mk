
dbus-configure:
	#cd dbus && autoreconf -fsi
	rm -f dbus/config.cache
	cd dbus && ./autogen.sh --prefix=/usr --host=$(ARCH)-linux \
	--disable-Werror --disable-selinux --disable-tests \
	--sysconfdir=/tmp \
	--localstatedir=/tmp/var \
	--disable-xml-docs \
	--without-x \
	--enable-systemd=no \
	--disable-asserts \
	--disable-checks \
	--disable-verbose-mode \
	--with-dbus-user="nobody" \
	--with-dbus-session-bus-connect-address="/tmp/var/run/dbus/dbussocket" \
	GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build/glib" \
	GLIB_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -I$(TOP)/expat/lib -UHAVE_SELINUX -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
	LDFLAGS="$(LDLTO) -ffunction-sections -L$(TOP)/expat/dynamic/lib/.libs -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	#cd dbus && ./configure --prefix=/usr --host=$(ARCH)-linux
	# probabaly need --enable-systemd to add systemd support to start dbus, need: sudo apt install

dbus:
	$(MAKE) -C dbus
	#$(MAKE) -C dbus DESTDIR=$(TOP)/dbus/staged install

dbus-install:
	install -D dbus/bus/.libs/dbus-daemon $(INSTALLDIR)/dbus/usr/sbin/dbus-daemon
	install -D dbus/tools/.libs/dbus-launch $(INSTALLDIR)/dbus/usr/sbin/dbus-launch
	install -D dbus/tools/.libs/dbus-uuidgen $(INSTALLDIR)/dbus/usr/sbin/dbus-uuidgen

	# this is the dbus-daemon wrapper should not be installed!
	#install -D dbus/bus/dbus-daemon $(INSTALLDIR)/dbus/usr/sbin/dbus-daemon.sh

	#should not be necessary as dbus is laucned as a session service
	#install -D dbus/bus/.libs/dbus-daemon-launch-helper $(INSTALLDIR)/dbus/usr/sbin/dbus-daemon-launch-helper
	
	#this is the wrapper file should not be installed
	#install -D dbus/bus/dbus-daemon-launch-helper $(INSTALLDIR)/dbus/usr/sbin/dbus-daemon-launch-helper
	
	#these scripts are not necessary anymore since the actual dbus-daemon binary is installed instead of thw wrapper
	#install -c dbus/dbuslaunch $(INSTALLDIR)/dbus/usr/sbin
	#install -c dbus/dbus-avahi-launch $(INSTALLDIR)/dbus/usr/sbin
	
	install -D dbus/dbus/.libs/libdbus-1.so.3.32.0 $(INSTALLDIR)/dbus/usr/lib/libdbus-1.so.3.32.0
	cd $(INSTALLDIR)/dbus/usr/lib && \
		ln -sf libdbus-1.so.3.32.0 libdbus-1.so && \
		ln -sf libdbus-1.so.3.32.0 libdbus-1.so.3


dbus-clean:
	-$(MAKE) -C dbus clean

.PHONY: dbus-configure dbus dbus-install dbus-clean
