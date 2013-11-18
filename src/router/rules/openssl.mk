export OPENSSL_TARGET := linux-openwrt
ifeq ($(ARCH),armeb)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes_core.o aes-armv4.o aes_cbc.o"
export OPENSSL_CMAKEFLAGS := -DASMAES512   -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(ARCH),arm)
export OPENSSL_TARGET := linux-armv4
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes_core.o aes-armv4.o aes_cbc.o"
export OPENSSL_CMAKEFLAGS := -DASMAES512   -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(ARCH),mips)
export OPENSSL_TARGET := linux-mips
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes_core.o aes_cbc.o aes-mips.o"
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(ARCH),mipsel)
export OPENSSL_TARGET := linux-openwrt
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes_core.o aes_cbc.o aes-mips.o"
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(ARCH),powerpc)
export OPENSSL_TARGET := linux-ppc
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections  -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGESTS -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),i386)
export OPENSSL_TARGET := linux-openwrt
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes_core.o aes-586.o aes_cbc.o"
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections
endif

openssl:
	$(MAKE) -j 4 -C openssl build_apps CC="$(CC) -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -j 4 -C openssl build-shared CC="$(CC) -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -j 4 -C openssl CC="$(CC) -I$(TOP)/zlib -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	rm -f openssl/apps/openssl

openssl-shared: openssl
	$(MAKE) -j 4 -C openssl build-shared CC="$(CC) -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)


openssl-apps: openssl-shared	
	-rm openssl/apps/openssl
	$(MAKE) -j 4 -C openssl build_apps CC="$(CC) -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-apps-static:
	-rm openssl/libcrypto.so.1.0.0
	-rm openssl/libssl.so.1.0.0
	-rm openssl/apps/openssl
	$(MAKE) -j 4 -C openssl build_apps CC="$(CC) -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-install:
#ifeq ($(CONFIG_MADWIFI),y)
	-install -D openssl/libcrypto.so.1.0.0 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.1.0.0 
	-install -D openssl/libssl.so.1.0.0 $(INSTALLDIR)/openssl/usr/lib/libssl.so.1.0.0
#endif
#ifneq ($(ARCH),mipsel)
#	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
#endif
ifeq ($(CONFIG_FREERADIUS),y)
	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
endif
	@true

openssl-clean:
	$(MAKE) -C openssl clean


OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-sha0 no-smime \
					no-rmd160 no-aes192 no-ripemd no-camellia no-ans1 no-krb5
ifeq ($(CONFIG_XSCALE),y)
OPENSSL_OPTIONS:= no-ec no-err no-hw threads zlib-dynamic \
					no-sse2 no-perlasm -DHAVE_CRYPTODEV
else
OPENSSL_OPTIONS:= no-ec no-err no-hw threads zlib-dynamic \
					no-engines no-perlasm
endif



openssl-configure:
	cd openssl && ./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			$(COPTS) $(OPENSSL_CMAKEFLAGS) -DNDEBUG \
			$(TARGET_LDFLAGS) -ldl \
			-DOPENSSL_SMALL_FOOTPRINT \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS) \
	
