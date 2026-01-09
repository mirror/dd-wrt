libedit-configure:
	cd libedit && autoreconf -vfi
	cd libedit && ./configure --host=$(ARCH)-linux-uclibc --prefix=/usr --libdir=/usr/lib \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	LDFLAGS="-L$(TOP)/ncurses/lib"

libedit:
	make -C libedit

libedit-install:
	make -C libedit install DESTDIR=$(INSTALLDIR)/libedit
	rm -rf $(INSTALLDIR)/libedit/usr/share
	rm -rf $(INSTALLDIR)/libedit/usr/include
	rm -rf $(INSTALLDIR)/libedit/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/libedit/usr/lib/*.a
	rm -f $(INSTALLDIR)/libedit/usr/lib/*.la
