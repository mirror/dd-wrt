libgd: libpng minidlna zlib
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)   -I$(TOP)/minidlna/libjpeg-turbo-2.1.4 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/minidlna/libjpeg-turbo-2.1.4 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -lm -L$(TOP)/zlib -L$(TOP)/libpng/.libs -lpng16 -L$(TOP)/minidlna/lib -ljpeg -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libgd

libgd-clean:
	make -C libgd clean
	
libgd-configure: minidlna-configure minidlna libpng-configure libpng zlib-configure zlib
	cd libgd && autoreconf -fi && ./configure --host=$(ARCH)-linux-uclibc  \
	--with-jpeg=$(TOP)/minidlna/libjpeg-turbo-2.1.4 \
	--without-xpm \
	--without-x \
	--without-tiff \
	--without-freetype \
	--without-fontconfig \
	--without-x \
	--disable-shared \
	--disable-werror \
	--enable-static \
	--with-zlib \
	enable_werror=no \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/minidlna/libjpeg-turbo-2.1.4 -I$(TOP)/zlib" \
	LDFLAGS="-L$(TOP)/minidlna/lib -L$(TOP)/zlib -L$(TOP)/libpng/.libs -lpng16" \
	LIBZ_CFLAGS="-I$(TOP)/zlib" \
	LIBZ_LIBS="-L$(TOP)/zlib -lz" \
	LIBPNG_CFLAGS="-I$(TOP)/libpng" \
	LIBPNG_LIBS="-L$(TOP)/libpng/.libs -lpng16"

	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)   -I$(TOP)/minidlna/libjpeg-turbo-2.1.4 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/minidlna/libjpeg-turbo-2.1.4 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -lm -L$(TOP)/zlib -L$(TOP)/libpng/.libs -lpng16 -L$(TOP)/minidlna/lib -ljpeg -fPIC -v -Wl,--verbose" \
	LIBZ_CFLAGS="-I$(TOP)/zlib" \
	LIBZ_LIBS="-L$(TOP)/zlib -lz" \
	LIBPNG_CFLAGS="-I$(TOP)/libpng" \
	LIBPNG_LIBS="-L$(TOP)/libpng/.libs -lpng16" \
	$(MAKE) -C libgd

libgd-install:
	@true
