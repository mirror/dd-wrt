#export samba_cv_CC_NEGATIVE_ENUM_VALUES=no

samba3-configure:
#	cd samba3/source3 && ./configure --prefix=/usr --host=$(ARCH)-linux CC="$(ARCH-linux-uclibc-gcc)" CFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" CPPFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC"

samba3:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(MAKE) -C samba3/source all bin/smbpasswd  

samba3-install:
	mkdir -p $(INSTALLDIR)/samba3
	install -D samba3/source/bin/smbd $(INSTALLDIR)/samba3/usr/sbin/smbd
	install -D samba3/source/bin/nmbd $(INSTALLDIR)/samba3/usr/sbin/nmbd
	install -D samba3/source/bin/smbpasswd $(INSTALLDIR)/samba3/usr/sbin/smbpasswd


samba3-clean:
	$(MAKE) -C samba3/source clean
