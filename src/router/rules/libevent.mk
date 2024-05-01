
libevent:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/openssl" \
	$(MAKE) -C libevent

libevent-install:
	$(MAKE) -C libevent install DESTDIR=$(INSTALLDIR)/libevent
	rm -rf $(INSTALLDIR)/libevent/usr/bin
	rm -rf $(INSTALLDIR)/libevent/usr/include
	rm -rf $(INSTALLDIR)/libevent/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/libevent/usr/lib/*.a
	rm -f $(INSTALLDIR)/libevent/usr/lib/*.la


libevent-clean:
	$(MAKE) -C libevent clean

libevent-configure: openssl
	cd libevent && ./configure  --disable-debug-mode --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/openssl" 
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/openssl" \
	$(MAKE) -C libevent
