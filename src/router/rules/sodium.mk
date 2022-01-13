libsodium:
	make -C libsodium
	
libsodium-clean:
	make -C libsodium clean
	
libsodium-configure:
	cd libsodium && ./autogen.sh && ./configure --host=$(ARCH)-linux-uclibc  \
	--disable-ssp \
	--disable-shared \
	--enable-static \
	--enable-minimal \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(LTO) $(LTOFIXUP) $(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(LTO) $(LTOFIXUP) $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) $(COPTS) $(MIPS16_OPT) $(LTOFIXUP) -ffunction-sections -fdata-sections -Wl,--gc-sections  -fPIC -v -Wl,--verbose" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	make -C libsodium clean
	make -C libsodium

libsodium-install:
	@true