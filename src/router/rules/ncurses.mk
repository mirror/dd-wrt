ncurses-configure:
	cd ncurses && ./configure --host=$(ARCH)-linux-uclibc --with-shared CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -fPIC" \
		--enable-echo \
		--libdir=/usr/lib \
		--enable-const \
		--enable-overwrite \
		--disable-rpath \
		--disable-termcap \
		--disable-rpath-hack \
		--without-ada \
		--without-cxx \
		--without-cxx-binding \
		--without-debug \
		--without-profile \
		--without-progs \
		--with-normal \
		--with-shared \
		--enable-widec \
		--disable-lib-suffixes \
		--with-terminfo-dirs=/etc/terminfo \
		--with-default-terminfo-dir=/etc/terminfo 
	chmod a+x "$(TOP)/ncurses/misc/ncurses-config"
	make -C ncurses

ncurses-clean:
	make -C ncurses clean

ncurses:
	make -C ncurses
	rm -rf ncurses/include/ncurses
	mkdir -p ncurses/include/ncurses
	cp -a ncurses/include/*.h ncurses/include/ncurses

ncurses-install:
	make -C ncurses install.libs DESTDIR=$(INSTALLDIR)/ncurses
	rm -rf $(INSTALLDIR)/ncurses/usr/include
	rm -rf $(INSTALLDIR)/ncurses/usr/bin
	rm -f $(INSTALLDIR)/ncurses/usr/lib/*.a
	rm -f $(INSTALLDIR)/ncurses/usr/lib/libmenu*
ifneq ($(CONFIG_IPTRAF),y)
	rm -f $(INSTALLDIR)/ncurses/usr/lib/libpanel*
endif

