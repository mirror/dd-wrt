dropbear: zlib
	cd dropbear && ./configure --host=$(ARCH)-linux --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-libutil CC=$(ARCH)-linux-uclibc-gcc CFLAGS="-I../zlib $(COPTS) -L../zlib" LDFLAGS="-L../zlib" host_alias=$(ARCH)-linux
	$(MAKE) -C dropbear PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" SCPPROGRESS=1 MULTI=1

dropbear-install:
	@true
ifeq ($(CONFIG_DROPBEAR_SSHD),y)
	install -D dropbear/config/sshd.webservices $(INSTALLDIR)/dropbear/etc/config/sshd.webservices
endif

