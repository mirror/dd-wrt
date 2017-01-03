libmcrypt:
	CC="ccache $(ARCH)-linux-uclibc-gcc"
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libmcrypt
	
libmcrypt-install:
	mkdir -p $(INSTALLDIR)/libmcrypt/usr/lib ; true
	cp -av libmcrypt/lib/.libs/libmcrypt.so* $(INSTALLDIR)/libmcrypt/usr/lib/ ; true

libmcrypt-clean:
	make -C libmcrypt clean
	
libmcrypt-configure:
	cd libmcrypt &&   ./configure  ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH)  --enable-shared --enable-static CC="ccache $(ARCH)-linux-uclibc-gcc" CFLAGS="-fPIC -DNEED_PRINTF -Drpl_realloc=realloc -Drpl_malloc=malloc $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include" CPPFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include" 'LDFLAGS=-L$(TOP)/zlib'	
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libmcrypt
