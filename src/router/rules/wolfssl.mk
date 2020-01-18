wolfssl-configure:
	cd wolfssl && ./autogen.sh
	cd wolfssl && ./configure --prefix=/usr --host=$(ARCH)-linux --enable-opensslextra --disable-shared --enable-static -disable-errorstrings --disable-oldtls --disable-poly1305 --disable-chacha --enable-ecc --disable-sslv3 --enable-des3 --enable-md4 --enable-stunnel --enable-session-ticket --enable-wpas --enable-cmac CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

wolfssl:
	$(MAKE) -j 4 -C wolfssl

wolfssl-clean:
	if test -e "wolfssl/Makefile"; then make -C wolfssl clean; fi
	@true

wolfssl-install:
