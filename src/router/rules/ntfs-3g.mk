
ntfs-3g:
	CC="$(CC)" \
	CFLAGS="$(COPTS)  $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	$(MAKE) -C ntfs-3g/fuse
	$(MAKE) -C ntfs-3g all

ntfs-3g-install:
ifeq ($(CONFIG_LEGACY_KERNEL),y)
	install -D ntfs-3g/src/ntfs-3g $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfs-3g
endif
ifeq ($(CONFIG_NTFSPROGS),y)
	install -D ntfs-3g/ntfsprogs/mkntfs $(INSTALLDIR)/ntfs-3g/usr/sbin/mkfs.ntfs
	install -D ntfs-3g/ntfsprogs/ntfsfix $(INSTALLDIR)/ntfs-3g/usr/sbin/ntfsfix
endif
	@true

ntfs-3g-clean:
	$(MAKE) -C ntfs-3g/fuse clean
	$(MAKE) -C ntfs-3g clean

ntfs-3g-configure:
	cd ntfs-3g/fuse && ./configure --prefix=/usr \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			CC="$(CC)" \
			CXXFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB)   -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB)   -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) $(LDLTO) $(MIPS16_OPT) $(THUMB) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			AR_FLAGS="cru $(LTOPLUGIN)" \
			RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	make -C ntfs-3g/fuse
	cd ntfs-3g && autoreconf --install
	cd ntfs-3g && ./configure --prefix=/usr \
			--with-fuse=external \
			--target=$(ARCH)-linux \
			--host=$(ARCH) \
			--disable-crypto \
			CC="$(CC)" \
			FUSE_MODULE_CFLAGS="-D_FILE_OFFSET_BITS=64 -I$(TOP)/ntfs-3g/fuse/include" \
			FUSE_MODULE_LIBS="-pthread -L$(TOP)/ntfs-3g/fuse/lib/.libs -lfuse -lrt -ldl" \
			CXXFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB)   -ffunction-sections -fdata-sections -Wl,--gc-sections"  \
			CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB)   -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			LDFLAGS="$(COPTS) $(LDLTO) $(MIPS16_OPT) $(THUMB) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			AR_FLAGS="cru $(LTOPLUGIN)" \
			RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
