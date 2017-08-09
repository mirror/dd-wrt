export OPENSSL_TARGET := linux-generic32
ifeq ($(ARCH),armeb)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512   -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif


ifeq ($(ARCHITECTURE),ventana)
ifeq ($(ARCH),arm)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
else
ifeq ($(ARCH),arm)
ifeq ($(CONFIG_STORM),y)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT

else
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
endif
endif
ifeq ($(ARCH),mips)
export OPENSSL_TARGET := linux-mips32
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),mips64)
export OPENSSL_TARGET := linux-mips64
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOCTEON -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGESTS -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),mipsel)
export OPENSSL_TARGET := linux-mips32
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),powerpc)
export OPENSSL_TARGET := linux-generic32
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),i386)
export OPENSSL_TARGET := linux-x86
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif
ifeq ($(ARCH),x86_64)
export OPENSSL_TARGET := linux-x86_64
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_SMALL_FOOTPRINT
endif


openssl:
	$(MAKE) -C openssl CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -C openssl build_libs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE)  -C openssl build_programs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	rm -f openssl/apps/openssl

openssl-shared: openssl
	$(MAKE) -C openssl build_libs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)


openssl-apps: openssl-shared	
	-rm openssl/apps/openssl
	$(MAKE) -C openssl build_programs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-apps-static:
	-rm openssl/libcrypto.so.1.1
	-rm openssl/libssl.so.1.1
	-rm openssl/apps/openssl
	$(MAKE) -C openssl build_programs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-install:
#ifeq ($(CONFIG_MADWIFI),y)
	-install -D openssl/libcrypto.so.1.1 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.1.1
	-install -D openssl/libssl.so.1.1 $(INSTALLDIR)/openssl/usr/lib/libssl.so.1.1
#endif
#ifneq ($(ARCH),mipsel)
#	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
#endif
ifeq ($(CONFIG_FREERADIUS),y)
	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
	-mkdir -p $(INSTALLDIR)/openssl/etc/ssl
	-touch $(INSTALLDIR)/openssl/etc/ssl/openssl.cnf
endif
	@true

openssl-clean:
	$(MAKE) -C openssl clean



OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-camellia no-whirlpool no-seed

OPENSSL_OPTIONS:= no-err threads no-ssl2 enable-ssl3 enable-ssl3-method zlib-dynamic no-ec2m no-heartbeats no-async no-egd

ifeq ($(CONFIG_XSCALE),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV
endif
ifeq ($(CONFIG_STORM),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV
endif
ifeq ($(CONFIG_VENTANA),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV
endif
ifeq ($(CONFIG_MVEBU),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGEST
endif
ifeq ($(CONFIG_ALPINE),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGEST
endif



openssl-configure:
	cd openssl && CROSS_COMPILE= && ./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			$(COPTS) $(OPENSSL_CMAKEFLAGS)  -L$(TOP)/zlib -I$(TOP)/zlib -DNDEBUG \
			$(TARGET_LDFLAGS) -ldl \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS)

	$(MAKE) -C openssl clean

	-$(MAKE) -C openssl CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_libs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_programs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-rm -f openssl/apps/openssl
	
	-$(MAKE) -C openssl CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_libs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_programs CC="$(CC) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-rm -f openssl/apps/openssl
