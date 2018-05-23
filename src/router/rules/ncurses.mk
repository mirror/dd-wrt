ncurses-configure:
	cd ncurses && ./configure --host=$(ARCH)-linux-uclibc --with-shared CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" \
		--enable-echo \
		--libdir=/usr/lib \
		--enable-const \
		--enable-overwrite \
		--disable-rpath \
		--without-ada \
		--without-cxx \
		--without-cxx-binding \
		--without-debug \
		--without-profile \
		--without-progs \
		--with-normal \
		--with-shared \
		--with-terminfo-dirs=/etc/terminfo \
		--with-default-terminfo-dir=/etc/terminfo 

ncurses-clean:
	make -j 4 -C ncurses clean

ncurses:
	make -j 4 -C ncurses

ncurses-install:
	make -C ncurses install.libs DESTDIR=$(INSTALLDIR)/ncurses
	rm -rf $(INSTALLDIR)/ncurses/usr/include
	rm -rf $(INSTALLDIR)/ncurses/usr/bin
	rm -f $(INSTALLDIR)/ncurses/usr/lib/*.a
	rm -f $(INSTALLDIR)/ncurses/usr/lib/libmenu*
ifneq ($(CONFIG_IPTRAF),y)
	rm -f $(INSTALLDIR)/ncurses/usr/lib/libpanel*
endif

