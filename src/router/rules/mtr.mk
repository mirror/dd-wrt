mtr-configure:
	cd mtr && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -lncurses" --without-gtk ac_cv_lib_resolv_res_mkquery=yes --disable-ipv6 --with-ncurses="$(TOP)/ncurses"



mtr: ncurses
	$(MAKE) -j 4 -C mtr

mtr-clean:
	if test -e "mtr/Makefile"; then make -C mtr clean; fi
	@true

mtr-install:
	$(MAKE) -C mtr install DESTDIR=$(INSTALLDIR)/mtr
	rm -rf $(INSTALLDIR)/mtr/usr/share
