curl:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypo -lssl" \
	$(MAKE) -C curl

curl-install:
	$(MAKE) -C curl install DESTDIR=$(INSTALLDIR)/curl
	rm -f $(INSTALLDIR)/curl/usr/bin/curl-config
	rm -rf $(INSTALLDIR)/curl/usr/include
	rm -rf $(INSTALLDIR)/curl/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/curl/usr/lib/*.a
	rm -f $(INSTALLDIR)/curl/usr/lib/*.la
	rm -rf $(INSTALLDIR)/curl/usr/share

curl-clean:
	$(MAKE) -C curl clean

curl-configure:
	cd curl && ./configure --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib  -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypto -lssl" 
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/zlib -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib -L$(TOP)/openssl -lcrypo -lssl" \
	$(MAKE) -C curl



libevent:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/openssl" \
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

libevent-configure:
	cd libevent && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/openssl" 
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/openssl" \
	$(MAKE) -C libevent



transmission: libevent curl
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include  -I$(TOP)/openssl/include  -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/zlib    -L$(TOP)/openssl -L$(TOP)/libevent/.libs -L$(TOP)/curl/lib/.libs" \
	$(MAKE) -C transmission

transmission-install:
	$(MAKE) -C transmission install DESTDIR=$(INSTALLDIR)/transmission
	rm -rf $(INSTALLDIR)/transmission/usr/share/man

transmission-clean:
	$(MAKE) -C transmission clean

transmission-configure: libevent-configure curl-configure
	cd transmission && ./configure  --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	--enable-daemon \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib   -I$(TOP)/curl/include -I$(TOP)/openssl/include -I$(TOP)/libevent/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -I$(TOP)/zlib  -I$(TOP)/curl/include  -I$(TOP)/openssl/include  -I$(TOP)/libevent/include" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT)  -L$(TOP)/zlib   -L$(TOP)/openssl -L$(TOP)/libevent/.libs  -L$(TOP)/curl/lib/.libs" 