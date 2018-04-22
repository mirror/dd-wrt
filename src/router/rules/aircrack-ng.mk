aircrack-ng: 
	$(MAKE) -C aircrack-ng

aircrack-ng-install:
	@true

aircrack-ng-configure:
	cd aircrack-ng && ./autogen.sh
	cd aircrack-ng && ./configure --host=$(ARCH)-linux \
	--with-openssl="$(TOP)/openssl" \
        --prefix=/usr \
	ZLIB_CFLAGS="-I$(TOP)/zlib" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	PCRE_CFLAGS="-I$(TOP)/pcre" \
	PCRE_LIBS="-L$(TOP)/pcre/.libs -lpcre" \
        OPENSSL_LDFLAGS="-L$(TOP)/openssl" \
        OPENSSL_LIBS="-lssl -lcrypto" \
        CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -std=gnu89 -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE" \
        CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -std=gnu89 -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -D_GNU_SOURCE" \
        LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/openssl -ffunction-sections -fdata-sections -Wl,--gc-sections"

aircrack-ng-clean:
	$(MAKE) -C aircrack-ng clean

