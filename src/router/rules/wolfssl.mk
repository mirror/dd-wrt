wolfssl-configure:
	cd wolfssl && ./configure --prefix=/usr --host=$(ARCH)-linux --enable-opensslextra --disable-shared --enable-static --disable-oldtls --enable-ecc --disable-sslv3 CC="ccache $(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections"

wolfssl:
	$(MAKE) -j 4 -C wolfssl

wolfssl-clean:
	if test -e "wolfssl/Makefile"; then make -C wolfssl clean; fi
	@true

wolfssl-install:
