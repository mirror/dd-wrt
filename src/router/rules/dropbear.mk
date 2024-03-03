DROPBEAR_OPTS = $(MIPS16_OPT) $(LTO) -DDISABLE_X11FWD

dropbear-configure: nvram libutils-configure libutils zlib-configure zlib
	cd dropbear && autoconf
	cd dropbear && ./configure \
				--host=$(ARCH)-linux \
				--disable-pam \
				--disable-harden \
				--disable-lastlog \
				--disable-utmp \
				--disable-zlib \
				--disable-utmpx \
				--disable-wtmp \
				--disable-wtmpx \
				--enable-bundled-libtom \
				--disable-pututxline \
				CC="$(CC)" \
				CPPFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -I../zlib $(COPTS) $(MIPS16_OPT) $(DROPBEAR_OPTS) -DARGTYPE=3 -DXFREE=free -L../zlib" \
				LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libutils -L$(TOP)/nvram -lshutils -lnvram" \
				host_alias=$(ARCH)-linux ac_cv_func_getpass=yes \
				AR_FLAGS="cru $(LTOPLUGIN)" \
				RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

dropbear: zlib
	install -D dropbear/config/sshd.webservices httpd/ej_temp/sshd.webservices
	$(MAKE) -C dropbear PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" SCPPROGRESS=1 MULTI=1

dropbear-install:
	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/sbin/dropbearmulti
	install -D dropbear/config/sshd.webservices $(INSTALLDIR)/dropbear/etc/config/sshd.webservices
	cd $(INSTALLDIR)/dropbear/usr/sbin && ln -sf /usr/sbin/dropbearmulti dropbearconvert && ln -sf /usr/sbin/dropbearmulti dropbearkey && ln -sf /usr/sbin/dropbearmulti dropbear
	mkdir -p $(INSTALLDIR)/dropbear/usr/bin
	cd $(INSTALLDIR)/dropbear/usr/bin && ln -sf /usr/sbin/dropbearmulti ssh && ln -sf /usr/sbin/dropbearmulti scp && ln -sf /usr/sbin/dropbearmulti dbclient 

