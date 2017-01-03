libgd: libpng minidlna
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/minidlna/jpeg-8 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/minidlna/jpeg-8 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -lm -L$(TOP)/zlib -L$(TOP)/libgd/libpng/.libs -lpng12 -L$(TOP)/minidlna/lib -ljpeg -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libgd
	
libgd-clean:
	make -C libgd clean
	
libgd-configure:
	cd libgd && ./configure --host=$(ARCH)-linux-uclibc  \
	--with-jpeg=$(TOP)/minidlna/jpeg-8 \
	--without-xpm \
	--without-x \
	--without-tiff \
	--without-freetype \
	--without-fontconfig \
	--without-x \
	--disable-shared \
	--enable-static \
	--with-zlib \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/minidlna/jpeg-8 -I$(TOP)/zlib" \
	LDFLAGS="-L$(TOP)/minidlna/lib -L$(TOP)/zlib -L$(TOP)/libgd/libpng/.libs -lpng12" \
	LIBPNG_CFLAGS="-I$(TOP)/libgd/libpng" \
	LIBPNG_LIBS="-L$(TOP)/libgd/libpng/.libs -lpng12"

	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/minidlna/jpeg-8 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/minidlna/jpeg-8 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -lm -L$(TOP)/zlib -L$(TOP)/libgd/libpng/.libs -lpng12 -L$(TOP)/minidlna/lib -ljpeg -fPIC -v -Wl,--verbose" \
	LIBPNG_CFLAGS="-I$(TOP)/libgd/libpng" \
	LIBPNG_LIBS="-L$(TOP)/libgd/libpng/.libs -lpng12" \
	$(MAKE) -C libgd

libgd-install:
	@true