samba:
	install -D samba/config/sambafs.webconfig httpd/ej_temp/sambafs.webconfig
	cd samba/cifs && $(ARCH)-linux-uclibc-gcc $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -o mount.cifs mount.cifs.c mtab.c util.c resolve_host.c

samba-install:
	install -D samba/config/sambafs.webconfig $(INSTALLDIR)/samba/etc/config/sambafs.webconfig
	install -D samba/config/sambafs.startup $(INSTALLDIR)/samba/etc/config/sambafs.startup
	install -D samba/config/sambafs.nvramconfig $(INSTALLDIR)/samba/etc/config/sambafs.nvramconfig
	install -D samba/cifs/mount.cifs $(INSTALLDIR)/samba/bin/mount.cifs


samba-clean:
	@true