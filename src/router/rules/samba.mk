samba:
	$(ARCH)-linux-uclibc-gcc $(COPTS) -DNEED_PRINTF -o samba/cifs/mount.cifs samba/cifs/mount.cifs.c

samba-install:
	install -D samba/config/sambafs.webconfig $(INSTALLDIR)/samba/etc/config/sambafs.webconfig
	install -D samba/config/sambafs.startup $(INSTALLDIR)/samba/etc/config/sambafs.startup
	install -D samba/config/sambafs.nvramconfig $(INSTALLDIR)/samba/etc/config/sambafs.nvramconfig
	install -D samba/cifs/mount.cifs $(INSTALLDIR)/samba/bin/mount.cifs


samba-clean:
	@true