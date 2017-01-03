libpng:
	cd libgd && \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libpng
	-mkdir -p $(TOP)/libgd/libpng/.libs/include
	-cp $(TOP)/libgd/libpng/*.h $(TOP)/libgd/libpng/.libs/include
	
libpng-clean:
	cd libgd && \
	make -C libpng clean
	
libpng-configure:
	cd libgd && \
	cd libpng &&   ./configure --host=$(ARCH)-linux-uclibc  --disable-shared --enable-static CC="ccache $(ARCH)-linux-uclibc-gcc" CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include" CPPFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include" 'LDFLAGS=-L$(TOP)/zlib'	
	cd libgd && \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT)   -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib -fPIC -v -Wl,--verbose" \
	$(MAKE) -C libpng
