
htop-configure: ncurses libnl
	cd htop && ./autogen.sh
	cd htop && sh ./configure --host=$(ARCH)-linux \
	    --prefix=/usr \
	    --enable-taskstats \
	    --enable-delayacct \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(LTO) -I$(TOP)/ncurses/include " \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(TOP)/ncurses/lib" \
	    HTOP_NCURSES_CONFIG_SCRIPT="$(TOP)/ncurses/misc/ncurses-config" \
	    LIBNL3_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL3_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3" \
	    LIBNL3GENL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL3GENL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-genl-3"

htop: ncurses libnl
	$(MAKE) -C htop

htop-clean:
	$(MAKE) -C htop clean

htop-install:
#	$(MAKE) -C htop installDESTDIR=$(INSTALLDIR)/htop
	install -D htop/htop $(INSTALLDIR)/htop/usr/sbin/htop
