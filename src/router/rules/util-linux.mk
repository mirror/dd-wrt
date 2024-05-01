util-linux-configure: ncurses
	-make -C util-linux clean

	cd util-linux && rm -f config.cache
	cd util-linux && libtoolize
	cd util-linux && aclocal
	cd util-linux && autoconf
	cd util-linux && autoheader
	cd util-linux && autoreconf -vfi
	cd util-linux && ./configure --host=$(ARCH)-linux-uclibc --prefix=/usr --libdir=/usr/tmp CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -DNEED_PRINTF -I$(TOP)/ncurses/include" PKG_CONFIG="/tmp" \
	--disable-rpath \
	--enable-new-mount	\
	--disable-tls		\
	--disable-sulogin	\
	--without-python	\
	--without-udev		\
	--disable-widechar	\
	--without-ncursesw	\
	--without-tinfo \
	--without-ncurses
	make -C util-linux

util-linux-clean:
	make -C util-linux clean

util-linux: ncurses
	make -C util-linux

util-linux-install:
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_E2FSPROGS),y)
ifneq ($(CONFIG_USBIP),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
