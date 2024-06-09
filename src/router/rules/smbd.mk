smbd-configure: libnl
	cd smbd/tools && ./autogen.sh
	cd smbd/tools && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static  --libdir=/usr/lib \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -DNEED_PRINTF -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd smbd/tools-glib && ./autogen.sh
	cd smbd/tools-glib && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static  --libdir=/usr/lib \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -DNEED_PRINTF -I$(TOP)/_staging_static/usr/include/glib-2.0 -I$(TOP)/_staging_static/usr/lib/glib-2.0/include -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="-L$(TOP)/_staging_static/usr/lib $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd smbd/tools-glib.61 && ./autogen.sh
	cd smbd/tools-glib.61 && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static  --libdir=/usr/lib \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -DNEED_PRINTF -I$(TOP)/_staging_static/usr/include/glib-2.0 -I$(TOP)/_staging_static/usr/lib/glib-2.0/include -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="-L$(TOP)/_staging_static/usr/lib $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

smbd: libnl
	install -D smbd/config/samba_ksmbd.webnas httpd/ej_temp/02samba_ksmbd.webnas
	install -D filesharing/config/zfilesharing.webnas httpd/ej_temp/03zfilesharing.webnas
ifneq ($(KERNELVERSION),6.1-nss)
ifneq ($(KERNELVERSION),6.6-nss)
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	$(MAKE) -C smbd/smbd all
	$(MAKE) -C smbd/tools all
else
	$(MAKE) -C smbd/smbd.61 all
	$(MAKE) -C smbd/tools-glib.61 all
endif
else
	$(MAKE) -C smbd/smbd.61 all
	$(MAKE) -C smbd/tools-glib.61 all
endif
else
	$(MAKE) -C smbd/smbd.61 all
	$(MAKE) -C smbd/tools-glib.61 all
endif
else
	$(MAKE) -C smbd/smbd.61 all
	$(MAKE) -C smbd/tools-glib.61 all
endif

smbd-install:
ifneq ($(KERNELVERSION),6.6-nss)
ifneq ($(KERNELVERSION),6.1-nss)
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	$(MAKE) -C smbd/smbd install
	$(MAKE) -C smbd/tools-glib install DESTDIR=$(INSTALLDIR)/smbd
	rm -rf $(INSTALLDIR)/smbd/usr/lib
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.mountd
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.adduser
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.addshare
else
	$(MAKE) -C smbd/smbd.61 install
	$(MAKE) -C smbd/tools-glib.61 install DESTDIR=$(INSTALLDIR)/smbd
endif
else
	$(MAKE) -C smbd/smbd.61 install
	$(MAKE) -C smbd/tools-glib.61 install DESTDIR=$(INSTALLDIR)/smbd
endif
else
	$(MAKE) -C smbd/smbd.61 install
	$(MAKE) -C smbd/tools-glib.61 install DESTDIR=$(INSTALLDIR)/smbd
endif
else
	$(MAKE) -C smbd/smbd.61 install
	$(MAKE) -C smbd/tools-glib.61 install DESTDIR=$(INSTALLDIR)/smbd
endif
	rm -rf $(INSTALLDIR)/smbd/usr/share
	rm -rf $(INSTALLDIR)/smbd/usr/etc
	rm -rf $(INSTALLDIR)/smbd/usr/lib/systemd

	install -D smbd/config/samba_ksmbd.webnas $(INSTALLDIR)/smbd/etc/config/02samba_ksmbd.webnas
	install -D smbd/config/samba_ksmbd.nvramconfig $(INSTALLDIR)/smbd/etc/config/samba_ksmbd.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/smbd/etc/config/03zfilesharing.webnas

smbd-clean:
ifneq ($(KERNELVERSION),6.6-nss)
ifneq ($(KERNELVERSION),6.1-nss)
ifneq ($(KERNELVERSION),6.1)
ifneq ($(KERNELVERSION),6.6)
	$(MAKE) -C smbd/smbd clean
	$(MAKE) -C smbd/tools clean
	$(MAKE) -C smbd/tools-glib clean
else
	$(MAKE) -C smbd/smbd.61 clean
	$(MAKE) -C smbd/tools-glib.61 clean
endif
else
	$(MAKE) -C smbd/smbd.61 clean
	$(MAKE) -C smbd/tools-glib.61 clean
endif
else
	$(MAKE) -C smbd/smbd.61 clean
	$(MAKE) -C smbd/tools-glib.61 clean
endif
else
	$(MAKE) -C smbd/smbd.61 clean
	$(MAKE) -C smbd/tools-glib.61 clean
endif
