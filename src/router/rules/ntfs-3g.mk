
ntfs-3g:
	CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS)" \
	CPPFLAGS="$(COPTS)" \
	LDFLAGS="$(COPTS) -fPIC" \
	$(MAKE) -C ntfs-3g all

ntfs-3g-install:
	install -D ntfs-3g/src/ntfs-3g $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g
	install -D ntfs-3g/src/ntfs-3g.probe $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g.probe
	$(STRIP) $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g
	$(STRIP) $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g.probe

ntfs-3g-clean:
	$(MAKE) -C ntfs-3g clean

ntfs-3g-configure:
	cd ntfs-3g && ./configure --prefix=/usr --with-fuse=$(TOP)/ntfs-3g/fuse --target=$(ARCH)-linux --host=$(ARCH) CC=$(ARCH)-linux-uclibc-gcc