
htop-configure: ncurses
	cd htop && ./autogen.sh
	cd htop && ./configure --host=$(ARCH)-linux \
	    --prefix=/usr --enable-taskstats \
	    CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(LTO) -I$(TOP)/ncurses/include" \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(TOP)/ncurses/lib"

htop: ncurses
	$(MAKE) -C htop

htop-clean:
	$(MAKE) -C htop clean

htop-install:
#	$(MAKE) -C htop installDESTDIR=$(INSTALLDIR)/htop
	install -D htop/htop $(INSTALLDIR)/htop/usr/sbin/htop
