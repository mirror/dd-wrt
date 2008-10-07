dropbear-configure: zlib
	cd dropbear && ./configure --host=$(ARCH)-linux --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-libutil CC=$(ARCH)-linux-uclibc-gcc CFLAGS="-I../zlib $(COPTS) -L../zlib -ffunction-sections -fdata-sections -Wl,--gc-sections " LDFLAGS="-L../zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" host_alias=$(ARCH)-linux

dropbear: zlib
	$(MAKE) -j 4 -C dropbear PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" SCPPROGRESS=1 MULTI=1

dropbear-install:
	@true
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/sbin/dropbearmulti
	install -D dropbear/config/sshd.webservices $(INSTALLDIR)/dropbear/etc/config/sshd.webservices
endif

