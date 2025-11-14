sed: 
	$(MAKE) -C sed

sed-install:
	$(MAKE) -C sed install DESTDIR=$(INSTALLDIR)/sed
	rm -rf $(INSTALLDIR)/sed/usr/share

sed-configure:
	cd sed && libtoolize
	cd sed && aclocal
	cd sed && autoconf
	cd sed && autoheader
	cd sed && automake --add-missing
	cd sed && ./configure \
	--host=$(ARCH)-linux \
	--enable-linux \
	--prefix=/usr \
	--disable-acl \
	--without-selinux \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"

sed-clean:
	$(MAKE) -C sed clean

