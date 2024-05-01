nocat-configure:
	rm -f $(TOP)/glib/config.cache
	cd glib && ./config.sh "$(CC)" "$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -std=gnu89 -ffunction-sections -fdata-sections -Wl,--gc-sections" "$(LTOPLUGIN)" ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH)
	cd nocat && ./configure \
	    --with-remote-splash \
	    CC="$(CC)" \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -std=gnu89 -DNEED_PRINTF -I../libghttp -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDCFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    --prefix=/tmp/ \
	    --with-glib-prefix=$(TOP)/glib-1.2.10-install \
	    --disable-glibtest \
	    --host=$(ARCH)-linux \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

nocat:
ifeq ($(CONFIG_RAMSKOV),y)
	install -D nocat/config_redirect/nocat.webhotspot httpd/ej_temp/nocat.webhotspot
else
	install -D nocat/config/nocat.webhotspot httpd/ej_temp/nocat.webhotspot
endif


	make  -C glib
	cp $(TOP)/glib/.libs/*.a $(TOP)/glib-1.2.10-install/lib
	cp $(TOP)/glib/gmodule/.libs/*.a $(TOP)/glib-1.2.10-install/lib
	cp $(TOP)/glib/gthread/.libs/*.a $(TOP)/glib-1.2.10-install/lib
	make  -C nocat

nocat-clean:
	make  -C glib clean
	make  -C nocat clean
	

nocat-install:
	install -D nocat/src/splashd $(INSTALLDIR)/nocat/usr/sbin/splashd
	$(STRIP) $(INSTALLDIR)/nocat/usr/sbin/splashd
	mkdir -p ${INSTALLDIR}/nocat/etc
	ln -sf /tmp/etc/nocat.conf $(INSTALLDIR)/nocat/etc/nocat.conf
	mkdir -p $(INSTALLDIR)/nocat/usr/libexec
	cp -r nocat/libexec/iptables $(INSTALLDIR)/nocat/usr/libexec/nocat
	rm -rf $(INSTALLDIR)/nocat/usr/libexec/nocat/iptables
ifeq ($(CONFIG_RAMSKOV),y)
	install -D nocat/config_redirect/nocat.webhotspot $(INSTALLDIR)/nocat/etc/config/nocat.webhotspot
	install -D nocat/config_redirect/nocat.nvramconfig $(INSTALLDIR)/nocat/etc/config/nocat.nvramconfig
	install -D nocat/config_redirect/nocat.startup $(INSTALLDIR)/nocat/etc/config/nocat.startup
	install -D nocat/config_redirect/nocat.header $(INSTALLDIR)/nocat/etc/config/nocat.header
	install -D nocat/config_redirect/nocat.footer $(INSTALLDIR)/nocat/etc/config/nocat.footer
else
	install -D nocat/config/nocat.webhotspot $(INSTALLDIR)/nocat/etc/config/nocat.webhotspot
	install -D nocat/config/nocat.nvramconfig $(INSTALLDIR)/nocat/etc/config/nocat.nvramconfig
	install -D nocat/config/nocat.startup $(INSTALLDIR)/nocat/etc/config/nocat.startup
endif
