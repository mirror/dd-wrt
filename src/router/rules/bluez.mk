
bluez-configure:
	#cd bluez && autoreconf -fsi
	rm -f bluez/config.cache
	cd bluez && ./configure --prefix=/usr --host=$(ARCH)-linux \
	--enable-static \
	--enable-client \
	--enable-datafiles \
	--enable-experimental \
	--enable-library \
	--enable-monitor \
	--enable-obex \
	--enable-threads \
	--enable-tools \
	--disable-android \
	--disable-cups \
	--disable-manpages \
	--disable-sixaxis \
	--disable-systemd \
	--disable-test \
	--disable-udev \
	--enable-deprecated \
	GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -I$(TOP)/glib20/libglib/build/glib" \
	GLIB_LIBS="-L$(TOP)/glib20/libglib -L$(TOP)/glib20/libglib/glib -L$(TOP)/glib20/libglib/build/glib" \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -UHAVE_SELINUX -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	#cd dbus && ./configure --prefix=/usr --host=$(ARCH)-linux
	# probabaly need --enable-systemd to add systemd support to start dbus, need: sudo apt install

bluez:
	$(MAKE) -C bluez

bluez-install:
	$(MAKE) -C bluez DESTDIR=$(TOP)/bluez install


bluez-clean:
	-$(MAKE) -C bluez clean

.PHONY: bluez-configure bluez bluez-install bluez-clean
