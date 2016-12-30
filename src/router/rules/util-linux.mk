util-linux-configure:
	make -C util-linux clean
	cd util-linux && ./configure --host=$(ARCH)-linux-uclibc --prefix=/usr --libdir=/usr/lib CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" PKG_CONFIG="/tmp" NCURSES_CFLAGS="-I$(TOP)/ncurses/include" NCURSES_LIBS="-L$(TOP)/ncurses/lib -lncurses" \
	--disable-rpath \
	--enable-new-mount	\
	--disable-tls		\
	--disable-sulogin	\
	--without-python	\
	--without-udev		\
	--with-ncurses
	make -C util-linux

util-linux-clean:
	make -C util-linux clean

util-linux:
	make -C util-linux

util-linux-install:
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
ifneq ($(CONFIG_ASTERISK),y)
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*
endif
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
