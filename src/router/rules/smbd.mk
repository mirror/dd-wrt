libnl-configure:
	cd libnl && autoreconf --install --verbose
	cd libnl && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static --disable-debug --enable-cli=no \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

libnl:
	$(MAKE) -C libnl

libnl-clean:
	$(MAKE) -C libnl clean

libnl-install:
	@true

smbd-configure: glib20 libnl
	cd smbd/tools && ./autogen.sh
	cd smbd/tools && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -D_GNU_SOURCE -DNEED_PRINTF -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -L$(INSTALLDIR)/util-linux/usr/lib" \
	    GLIB_LIBS="-L$(TOP)/glib20/libglib/glib/.libs -lglib-2.0" \
	    LIBNL_CFLAGS="-I$(TOP)/libnl/include" \
	    LIBNL_LIBS="-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

smbd: glib20 libnl
	$(MAKE) -C smbd/smbd all
	$(MAKE) -C smbd/tools all

smbd-install:
	$(MAKE) -C smbd/smbd install
	$(MAKE) -C smbd/tools install DESTDIR=$(INSTALLDIR)/smbd
	rm -rf $(INSTALLDIR)/smbd/usr/lib
	install -D samba4/config/samba4.webnas $(INSTALLDIR)/samba4/etc/config/02samba4.webnas
	install -D samba4/config/samba4.nvramconfig $(INSTALLDIR)/samba4/etc/config/samba4.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/samba4/etc/config/03zfilesharing.webnas

smbd-clean:
	$(MAKE) -C smbd/smbd clean
	$(MAKE) -C smbd/tools clean
