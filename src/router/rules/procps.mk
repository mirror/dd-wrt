procps-configure:
	cd procps && ./autogen.sh
	cd procps && ./configure --disable-nls --disable-static --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
	NCURSES_CFLAGS="-I$(TOP)/ncurses/include" \
	NCURSES_LIBS="-L$(TOP)/ncurses/lib -lncurses" \
	CC="$(CC)" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO)  -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -Drpl_realloc=realloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

procps: pciutils
	$(MAKE) -C procps

procps-clean:
	if test -e "procps/Makefile"; then make -C procps clean; fi
	@true

procps-install:
	$(MAKE) -C procps install DESTDIR=$(INSTALLDIR)/procps
	rm -rf $(INSTALLDIR)/procps/usr/share
	rm -rf $(INSTALLDIR)/procps/usr/include
	rm -rf $(INSTALLDIR)/procps/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/procps/usr/lib/*.la
