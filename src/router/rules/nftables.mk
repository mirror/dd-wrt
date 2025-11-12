libnftnl-configure:
	cd libnftnl && libtoolize
	cd libnftnl && aclocal
	cd libnftnl && autoconf
	cd libnftnl && autoheader
	cd libnftnl && automake --add-missing
	cd libnftnl && ./configure --host=$(ARCH)-linux --prefix=/usr --disable-shared --enable-static --libdir=/usr/lib \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include " \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LIBMNL_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" \
		LIBMNL_LIBS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/libmnl/src/.libs -lmnl" \
		AR_FLAGS="'cru $(LTOPLUGIN)'" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

libnftnl-clean:
	-make -C libnftnl clean

libnftnl:
	make -C libnftnl

libnftnl-install:
        # So that generic rule does not take precedence
	@true
	
	
nftables-configure: libmnl libnftnl
	cd nftables && libtoolize
	cd nftables && aclocal
	cd nftables && autoconf
	cd nftables && autoheader
	cd nftables && automake --add-missing
	cd nftables && ./configure --host=$(ARCH)-linux --with-mini-gmp --disable-debug --disable-man-doc --disable-python --without-cli --prefix=/usr --libdir=/usr/lib --disable-shared --enable-static \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" \
		LIBNFTNL_CFLAGS="-I$(TOP)/libnftnl/include -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" \
		LIBNFTNL_LIBS="-L$(TOP)/libnftnl/src/.libs -lnftnl" \
		LIBMNL_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" \
		LIBMNL_LIBS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/libmnl/src/.libs -lmnl" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libnftnl/src/.libs" \
		AR_FLAGS="'cru $(LTOPLUGIN)'" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

nftables-new-clean:
	-make -C nftables clean

nftables:
	-make -C nftables

nftables-install:
	make -C nftables install DESTDIR=$(INSTALLDIR)/nftables
	rm -rf $(INSTALLDIR)/nftables/usr/include
	rm -rf $(INSTALLDIR)/nftables/usr/lib
	rm -rf $(INSTALLDIR)/nftables/usr/share

