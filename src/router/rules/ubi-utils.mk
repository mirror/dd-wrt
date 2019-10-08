ubi-utils-configure:
	cd ubi-utils && ./autogen.sh
	cd ubi-utils && ./configure --prefix=/usr --host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/lzo/include -L$(TOP)/lzo/src/.libs -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LZO_CFLAGS="-I$(TOP)/lzo/include" \
		LZO_LIBS="-L$(TOP)/lzo/src/.libs -llzo2" \
		ZLIB_CFLAGS="-I$(TOP)/zlib/include" \
		ZLIB_LIBS="-L$(TOP)/zlib -lz" \
		ZSTD_CFLAGS="-I$(TOP)/zstd/lib" \
		ZSTD_LIBS="-L$(TOP)/zstd/lib -lzstd"
		
ubi-utils:
	$(MAKE) -C ubi-utils

ubi-utils-clean:
	$(MAKE) -C ubi-utils clean

ubi-utils-install:
	$(MAKE) -C ubi-utils install DESTDIR=$(INSTALLDIR)/ubi-utils
	rm -rf $(INSTALLDIR)/ubi-utils/usr/share