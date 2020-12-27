smbd-configure: libnl
	cd smbd-next/tools && ./autogen.sh
	cd smbd-next/tools && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static  --libdir=/usr/lib \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -D_GNU_SOURCE -DNEED_PRINTF -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

smbd: libnl
	install -D smbd-next/config/samba_ksmbd.webnas httpd/ej_temp/02samba_ksmbd.webnas
	install -D filesharing/config/zfilesharing.webnas httpd/ej_temp/03zfilesharing.webnas
	$(MAKE) -C smbd-next/smbd all
	$(MAKE) -C smbd-next/tools all

smbd-install:
	$(MAKE) -C smbd-next/smbd install
	$(MAKE) -C smbd-next/tools install DESTDIR=$(INSTALLDIR)/smbd
	rm -rf $(INSTALLDIR)/smbd/usr/lib
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.mountd
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.adduser
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.addshare

	install -D smbd-next/config/samba_ksmbd.webnas $(INSTALLDIR)/smbd/etc/config/02samba_ksmbd.webnas
	install -D smbd-next/config/samba_ksmbd.nvramconfig $(INSTALLDIR)/smbd/etc/config/samba_ksmbd.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/smbd/etc/config/03zfilesharing.webnas

smbd-clean:
	$(MAKE) -C smbd-next/smbd clean
	$(MAKE) -C smbd-next/tools clean
