screen-configure: ncurses
	cd screen && autoreconf --force --install --symlink
	cd screen && ./configure --host=$(ARCH)-linux --enable-colors256 --with-sys-screenrc=/etc/screenrc CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -I$(TOP)/ncurses/include" LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(TOP)/ncurses/lib" --prefix=/usr ac_cv_safe_to_define___extensions__=no

screen: ncurses
	make   -C screen

screen-clean:
	make   -C screen clean

screen-install:
	make   -C screen install DESTDIR=$(INSTALLDIR)/screen
	rm -rf $(INSTALLDIR)/screen/usr/share/man
	rm -rf $(INSTALLDIR)/screen/usr/share/info
	rm -rf $(INSTALLDIR)/screen/usr/lib
	install -Dm644 $(TOP)/screen/etc/etcscreenrc $(INSTALLDIR)/screen/etc/screenrc
	install -Dm644 $(TOP)/screen/etc/screenrc $(INSTALLDIR)/screen/etc/skel/.screenrc
