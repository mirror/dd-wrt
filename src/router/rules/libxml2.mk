
libxml2: zlib
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C libxml2

libxml2-install:
	mkdir -p $(INSTALLDIR)/libxml2/usr/lib ; true
	cp -av libxml2/.libs/libxml2.so.* $(INSTALLDIR)/libxml2/usr/lib/ ; true
	
#	$(STRIP) $(INSTALLDIR)/zabbix/usr/sbin/zabbix_agentd

libxml2-clean:
	$(MAKE) -C libxml2 clean

libxml2-configure: zlib
	cd libxml2 && ./configure ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" --without-python 
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DOLD_LIBC_MODE -ffunction-sections -fdata-sections -Wl,--gc-section"
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/zlib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/zlib -lz -fPIC" \
	$(MAKE) -C libxml2
