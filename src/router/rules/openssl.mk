export OPENSSL_TARGET := linux-openwrt
ifeq ($(ARCH),armeb)
export OPENSSL_TARGET := linux-armv4
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
endif
ifeq ($(ARCH),arm)
export OPENSSL_TARGET := linux-armv4
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
endif
ifeq ($(ARCH),powerpc)
export OPENSSL_TARGET := linux-ppc
#export OPENSSL_MAKEFLAGS := AES_ASM_OBJ="aes-armv4.o aes_cbc.o"
endif
ifeq ($(ARCH),i386)
export OPENSSL_TARGET := linux-i386
#export OPENSSL_CMAKEFLAGS := -DOPENSSL_FIPS_AES_ASM=1 -DOPENSSL_BN_ASM_PART_WORDS 
endif

openssl:
	$(MAKE) -j 4 -C openssl CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -j 4 -C openssl build-shared CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	rm -f openssl/apps/openssl
	$(MAKE) -j 4 -C openssl build_apps CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-shared: openssl
	$(MAKE) -j 4 -C openssl build-shared CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)


openssl-apps: openssl-shared
	rm openssl/apps/openssl
	$(MAKE) -j 4 -C openssl build_apps CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-install:
ifeq ($(CONFIG_MADWIFI),y)
	install -D openssl/libcrypto.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.0.9.8 
	install -D openssl/libssl.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libssl.so.0.9.8
endif
ifeq ($(CONFIG_FREERADIUS),y)
	install -D openssl/libcrypto.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.0.9.8
	install -D openssl/libssl.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libssl.so.0.9.8
endif
ifneq ($(ARCH),mipsel)
	install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
endif
	@true

openssl-clean:
	$(MAKE) -C openssl clean


OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-sha0 no-smime \
					no-rmd160 no-aes192 no-ripemd no-camellia no-ans1 no-krb5
ifeq ($(CONFIG_XSCALE),y)
OPENSSL_OPTIONS:= no-ec no-err no-hw threads zlib-dynamic \
					no-sse2 no-perlasm --with-cryptodev
else
OPENSSL_OPTIONS:= no-ec no-err no-hw threads zlib-dynamic \
					no-engines no-sse2 no-perlasm
endif



openssl-configure:
	cd openssl && ./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			$(COPTS) $(OPENSSL_CMAKEFLAGS) \
			$(TARGET_LDFLAGS) -ldl \
			-DOPENSSL_SMALL_FOOTPRINT \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS) \
	
