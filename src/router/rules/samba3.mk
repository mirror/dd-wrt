
samba3:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(MAKE) -C samba3/source all bin/smbpasswd  

samba3-install:
	mkdir -p $(INSTALLDIR)/samba3
	install -D samba3/source/bin/smbd $(INSTALLDIR)/samba3/usr/sbin/smbd
	install -D samba3/source/bin/nmbd $(INSTALLDIR)/samba3/usr/sbin/nmbd
	install -D samba3/source/bin/smbpasswd $(INSTALLDIR)/samba3/usr/sbin/smbpasswd
	install -D samba3/config/samba3.webnas $(INSTALLDIR)/samba3/etc/config/samba3.webnas
	install -D samba3/config/samba3.startup $(INSTALLDIR)/samba3/etc/config/samba3.startup
	install -D samba3/config/samba3.nvramconfig $(INSTALLDIR)/samba3/etc/config/samba3.nvramconfig


samba3-clean:
	$(MAKE) -C samba3/source clean
