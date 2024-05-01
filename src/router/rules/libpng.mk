libpng: zlib
	cd libpng && \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)   -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -L$(TOP)/zlib -fPIC -v -Wl,--verbose"
	$(MAKE) -C libpng
	-mkdir -p $(TOP)/libpng/.libs/include
	-cp $(TOP)/libpng/*.h $(TOP)/libpng/.libs/include
	-cp $(TOP)/libpng/.libs/libpng16.a $(TOP)/libpng/.libs/libpng.a

libpng-clean:
	make -C libpng clean
	
libpng-configure: zlib-configure zlib
	cd libpng &&   ./configure --host=$(ARCH)-linux-uclibc  --disable-shared --enable-static CC="ccache $(ARCH)-linux-uclibc-gcc" CFLAGS="-fPIC $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib/include" CPPFLAGS="-fPIC $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib/include" 'LDFLAGS=-L$(TOP)/zlib'
	$(MAKE) -C libpng
	-mkdir -p $(TOP)/libpng/.libs/include
	-cp $(TOP)/libpng/*.h $(TOP)/libpng/.libs/include
	-cp $(TOP)/libpng/.libs/libpng16.a $(TOP)/libpng/.libs/libpng.a

libpng-install:
	@true
