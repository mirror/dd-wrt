ubi-utils-configure: zlib
	cd ubi-utils && ./autogen.sh
	cd ubi-utils && ./configure --prefix=/usr --host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/lzo/include -L$(TOP)/lzo/src/.libs -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LZO_CFLAGS="$(LTO) -I$(TOP)/lzo/include" \
		LZO_LIBS="$(LDLTO) -L$(TOP)/lzo/src/.libs -llzo2" \
		ZLIB_CFLAGS=" $(LTO) -I$(TOP)/zlib/include" \
		ZLIB_LIBS=" $(LDLTO) -L$(TOP)/zlib -lz" \
		ZSTD_CFLAGS="$(LTO) -I$(TOP)/zstd/lib" \
		ZSTD_LIBS="$(LDLTO) -L$(TOP)/zstd/lib -lzstd" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
ubi-utils: zlib
	$(MAKE) -C ubi-utils

ubi-utils-clean:
	$(MAKE) -C ubi-utils clean

ubi-utils-install:
	$(MAKE) -C ubi-utils install DESTDIR=$(INSTALLDIR)/ubi-utils
	rm -rf $(INSTALLDIR)/ubi-utils/usr/share