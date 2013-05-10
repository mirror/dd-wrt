DROPBEAR_OPTS = $(MIPS16_OPT) 

dropbear-configure: zlib
	cd dropbear && ./configure --host=$(ARCH)-linux --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-libutil CC="$(CC)" CFLAGS="-DNEED_PRINTF -I../zlib $(COPTS) $(DROPBEAR_OPTS) -L../zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L../zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" host_alias=$(ARCH)-linux

dropbear: zlib
	$(MAKE) -j 4 -C dropbear PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" SCPPROGRESS=1 MULTI=1

dropbear-install:
	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/sbin/dropbearmulti
	install -D dropbear/config/sshd.webservices $(INSTALLDIR)/dropbear/etc/config/sshd.webservices
	cd $(INSTALLDIR)/dropbear/usr/sbin && ln -sf /usr/sbin/dropbearmulti dropbearconvert && ln -sf /usr/sbin/dropbearmulti dropbearkey && ln -sf /usr/sbin/dropbearmulti dropbear
	mkdir -p $(INSTALLDIR)/dropbear/usr/bin
	cd $(INSTALLDIR)/dropbear/usr/bin && ln -sf /usr/sbin/dropbearmulti ssh && ln -sf /usr/sbin/dropbearmulti scp && ln -sf /usr/sbin/dropbearmulti dbclient 

