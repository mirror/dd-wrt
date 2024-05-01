libnl-configure:
	cd libnl && autoreconf --install --verbose
	cd libnl && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static --disable-debug --enable-cli=no \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="\"cru $(LTOPLUGIN)\"" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	$(MAKE) -C libnl

libnl:
	$(MAKE) -C libnl

libnl-clean:
	$(MAKE) -C libnl clean

libnl-install:
	@true


