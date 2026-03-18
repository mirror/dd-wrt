wavemon-configure: ncurses libnl
	cd wavemon && autoreconf -fi
	cd wavemon && ./configure --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/libnl/include -D_GNU_SOURCE -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF  -lncurses -L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" LDFLAGS="-L$(TOP)/libnl/lib/.libs -L$(TOP)/libnl/src/lib/.libs -lnl-3 -lnl-genl-3" LDLIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3 -lnl-route-3 -lnl-cli-3"


wavemon: ncurses
	$(MAKE) -C wavemon \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/libnl/include -D_GNU_SOURCE -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF  -lncurses -L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" LDFLAGS="-L$(TOP)/libnl/lib/.libs -L$(TOP)/libnl/src/lib/.libs -lnl-3 -lnl-genl-3" LDLIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3 -lnl-route-3 -lnl-cli-3"

wavemon-clean:
	if test -e "wavemon/Makefile"; then make -C wavemon clean; fi
	@true

wavemon-install:
	$(MAKE) -C wavemon install DESTDIR=$(INSTALLDIR)/wavemon
	rm -rf $(INSTALLDIR)/wavemon/usr/share

#		LIBNL3_CFLAGS="-I$(TOP)/libnl/include" \
#		LIBNL3_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3  -lnl-genl-3" \
