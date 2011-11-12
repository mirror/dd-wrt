ifeq ($(CONFIG_HOTPLUG2),y)
export SAMBA3_EXTRA:= -DHAVE_DDINOTIFY
endif


samba3:
	CC="$(CC)" \
	CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) $(SAMBA3_EXTRA)" \
	CPPFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) $(SAMBA3_EXTRA)" \
	LDFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) $(SAMBA3_EXTRA)" \
	$(MAKE) -C samba3/source all bin/smbpasswd WITH_LFS=yes

	CC="$(CC)" \
	CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC $(LTO) $(SAMBA3_EXTRA)" \
	CPPFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC $(LTO) $(SAMBA3_EXTRA)" \
	LDFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC $(LTO) $(SAMBA3_EXTRA)" \
	$(MAKE) -C samba3/source_nmbd all bin/nmbd WITH_LFS=yes

samba3-install:
	mkdir -p $(INSTALLDIR)/samba3
	install -D samba3/source/bin/smbd $(INSTALLDIR)/samba3/usr/sbin/smbd
ifeq ($(CONFIG_BUFFALO),y)
	install -D samba3/source_nmbd/bin/nmbd $(INSTALLDIR)/samba3/usr/sbin/nmbd
endif
ifeq ($(CONFIG_NMBD),y)
	install -D samba3/source_nmbd/bin/nmbd $(INSTALLDIR)/samba3/usr/sbin/nmbd
endif
	install -D samba3/source/bin/smbpasswd $(INSTALLDIR)/samba3/usr/sbin/smbpasswd
	install -D samba3/config/samba3.webnas $(INSTALLDIR)/samba3/etc/config/samba3.webnas
#	install -D samba3/config/samba3.startup $(INSTALLDIR)/samba3/etc/config/samba3.startup
	install -D samba3/config/samba3.nvramconfig $(INSTALLDIR)/samba3/etc/config/samba3.nvramconfig


samba3-clean:
	$(MAKE) -C samba3/source clean
	$(MAKE) -C samba3/source_nmbd clean
