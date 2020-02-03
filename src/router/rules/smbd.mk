libnl-configure:
	cd libnl && autoreconf --install --verbose
	cd libnl && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static --disable-debug --enable-cli=no \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	$(MAKE) -C libnl

libnl:
	$(MAKE) -C libnl

libnl-clean:
	$(MAKE) -C libnl clean

libnl-install:
	@true

smbd-configure: libnl
	cd smbd/tools && ./autogen.sh
	cd smbd/tools && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -D_GNU_SOURCE -DNEED_PRINTF -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

smbd: libnl
	$(MAKE) -C smbd/smbd all
	$(MAKE) -C smbd/tools all

smbd-install:
	$(MAKE) -C smbd/smbd install
	$(MAKE) -C smbd/tools install DESTDIR=$(INSTALLDIR)/smbd
	rm -rf $(INSTALLDIR)/smbd/usr/lib
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.mountd
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.adduser
	cd $(INSTALLDIR)/smbd/usr/sbin && ln -sf smbd_multicall ksmbd.addshare

	install -D smbd/config/samba_ksmbd.webnas $(INSTALLDIR)/smbd/etc/config/02samba_ksmbd.webnas
	install -D smbd/config/samba_ksmbd.nvramconfig $(INSTALLDIR)/smbd/etc/config/samba_ksmbd.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/smbd/etc/config/03zfilesharing.webnas

smbd-clean:
	$(MAKE) -C smbd/smbd clean
	$(MAKE) -C smbd/tools clean
