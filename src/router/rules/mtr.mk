mtr-configure: ncurses-configure ncurses
	cd mtr && autoreconf -fi
	cd mtr && ./configure --disable-nls --without-glib --without-gtk --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -D__GLIBC__ -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF  -lncurses" --without-gtk ac_cv_lib_resolv_res_mkquery=yes
	cd mtr && touch *


mtr: ncurses
	$(MAKE) -j 4 -C mtr

mtr-clean:
	if test -e "mtr/Makefile"; then make -C mtr clean; fi
	@true

mtr-install:
	$(MAKE) -C mtr install DESTDIR=$(INSTALLDIR)/mtr
	rm -rf $(INSTALLDIR)/mtr/usr/share
