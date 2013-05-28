
ntfs-3g:
	CC="$(CC)" \
	CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(MAKE) -C ntfs-3g all

ntfs-3g-install:
	install -D ntfs-3g/src/ntfs-3g $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g
#	install -D ntfs-3g/src/ntfs-3g.probe $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g.probe
	$(STRIP) $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g
#	$(STRIP) $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g.probe

ntfs-3g-clean:
	$(MAKE) -C ntfs-3g clean

ntfs-3g-configure:
	cd ntfs-3g/fuse && ./configure --prefix=/usr \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			CC="$(CC)" \
			CXXFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections"
	make -C ntfs-3g/fuse
	cd ntfs-3g && ./configure --prefix=/usr \
			--with-fuse=external \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			CC="$(CC)" \
			FUSE_MODULE_CFLAGS="-D_FILE_OFFSET_BITS=64 -I$(TOP)/ntfs-3g/fuse/include" \
			FUSE_MODULE_LIBS="-pthread -L$(TOP)/ntfs-3g/fuse/lib/.libs -lfuse -lrt -ldl" \
			CXXFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections"
