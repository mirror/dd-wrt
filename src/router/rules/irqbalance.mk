irqbalance-configure: libffi glib20
	cd irqbalance && ./autogen.sh
	cd irqbalance && ./configure --disable-numa --prefix=/usr \
		--with-libcap_ng=no \
		--with-systemd=no \
		--disable-numa \
		--with-irqbalance-ui \
		--enable-static=glib2 \
		--disable-thermal \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		GLIB2_CFLAGS="-I$(TOP)/_staging_static/usr/include/glib-2.0 -I$(TOP)/_staging_static/usr/lib/glib-2.0/include -L$(INSTALLDIR)/util-linux/usr/lib" \
		GLIB2_LIBS="-L$(TOP)/_staging_static/usr/lib -lglib-2.0" \
		NCURSESW_CFLAGS="-I$(TOP)/ncurses/include" \
		NCURSESW_LIBS="-L$(TOP)/ncurses/lib -lncurses" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) $(THUMB) -I$(TOP)/ncurses/include -DNEED_PRINTF" \
		LDFLAGS="$(LDLTO) -L$(TOP)/ncurses/lib -lncurses"

irqbalance: zlib glib20
	$(MAKE) -C irqbalance

irqbalance-clean: 
	if test -e "irqbalance/Makefile"; then $(MAKE) -C irqbalance clean ; fi

irqbalance-install: 
	$(MAKE) -C irqbalance install DESTDIR=$(INSTALLDIR)/irqbalance
	install -D irqbalance/config/irqbalance.webservices $(INSTALLDIR)/irqbalance/etc/config/irqbalance.webservices
	install -D irqbalance/config/irqbalance.nvramconfig $(INSTALLDIR)/irqbalance/etc/config/irqbalance.nvramconfig
	$(MAKE) -C irqbalance install DESTDIR=$(INSTALLDIR)/irqbalance
	mkdir -p $(INSTALLDIR)/irqbalance/etc/irqbalance
	rm -rf $(INSTALLDIR)/irqbalance/etc
	rm -rf $(INSTALLDIR)/irqbalance/usr/etc
	rm -rf $(INSTALLDIR)/irqbalance/usr/lib
	rm -rf $(INSTALLDIR)/irqbalance/usr/include
	rm -rf $(INSTALLDIR)/irqbalance/usr/share
	