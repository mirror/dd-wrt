inadyn-configure:
	cd inadynv2/libconfuse && ./autogen.sh
	cd inadynv2/libconfuse && ./configure --prefix=/usr \
					--disable-examples \
					--disable-shared \
					--enable-static \
					--host=$(ARCH)-linux \
					CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
					AR_FLAGS="cru $(LTOPLUGIN)" \
					RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	$(MAKE) -C inadynv2/libconfuse clean
	$(MAKE) -C inadynv2/libconfuse

	cd inadynv2 && ./autogen.sh

	mkdir -p $(TOP)/inadynv2/build
	mkdir -p $(TOP)/inadynv2/build_ssl
	
	cd inadynv2/build && ../configure --prefix=/usr \
		--enable-reduced \
		--disable-ssl \
		--localstatedir=/tmp \
		--disable-shared \
		--enable-static \
		--host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		confuse_CFLAGS="-I$(TOP)/inadynv2/libconfuse/src" \
		confuse_LIBS="-L$(TOP)/inadynv2/libconfuse/src/.libs -lconfuse" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	-cd inadynv2/build_ssl && ../configure --prefix=/usr \
		--disable-reduced \
		--enable-ssl \
		--enable-openssl \
		--disable-shared \
		--enable-static \
		--localstatedir=/tmp \
		--host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		OpenSSL_CFLAGS="-I$(TOP)/openssl/include" \
		OpenSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto" \
		confuse_CFLAGS="-I$(TOP)/inadynv2/libconfuse/src" \
		confuse_LIBS="-L$(TOP)/inadynv2/libconfuse/src/.libs -lconfuse" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

inadyn:
	$(MAKE) -C inadynv2/libconfuse
ifeq ($(CONFIG_OPENSSL),y)
	$(MAKE) -C inadynv2/build_ssl
else
	$(MAKE) -C inadynv2/build
endif

inadyn-install:
ifeq ($(CONFIG_OPENSSL),y)
	install -D inadynv2/build_ssl/src/inadyn $(INSTALLDIR)/inadyn/usr/sbin/inadyn
else
	install -D inadynv2/build/src/inadyn $(INSTALLDIR)/inadyn/usr/sbin/inadyn
endif

inadyn-clean:
	$(MAKE) -C inadynv2/libconfuse clean
	$(MAKE) -C inadynv2/build_ssl clean
	$(MAKE) -C inadynv2/build clean



