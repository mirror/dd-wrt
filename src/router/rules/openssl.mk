export OPENSSL_TARGET := linux-generic32
ifeq ($(ARCH),armeb)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif


ifeq ($(ARCHITECTURE),ventana)
ifeq ($(ARCH),arm)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
else
ifeq ($(ARCH),arm)
ifeq ($(CONFIG_STORM),y)
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 

else
export OPENSSL_TARGET := linux-armv4
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
endif
endif
ifeq ($(ARCH),mips)
export OPENSSL_TARGET := linux-mips32
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),mips64)
export OPENSSL_TARGET := linux64-mips64
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOCTEON -DOCTEON_OPENSSL -I$(TOP)/openssl/include/executive 
endif
ifeq ($(ARCH),mipsel)
export OPENSSL_TARGET := linux-mips32
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),powerpc)
export OPENSSL_TARGET := linux-generic32
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),i386)
export OPENSSL_TARGET := linux-x86
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),x86_64)
export OPENSSL_TARGET := linux-x86_64
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),aarch64)
export OPENSSL_TARGET := linux-aarch64
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif


openssl:
	$(MAKE) -C openssl CC="$(CC) -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -C openssl build_libs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE)  -C openssl build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	rm -f openssl/apps/openssl

openssl-shared: openssl
	$(MAKE) -C openssl build_libs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)


openssl-apps: openssl-shared	
	-rm openssl/apps/openssl
	$(MAKE) -C openssl build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-apps-static:
	-rm openssl/libcrypto.so.1.1
	-rm openssl/libssl.so.1.1
	-rm openssl/apps/openssl
	$(MAKE) -C openssl build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-install:
#ifeq ($(CONFIG_MADWIFI),y)
	-install -D openssl/libcrypto.so.1.1 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.1.1
	-install -D openssl/libssl.so.1.1 $(INSTALLDIR)/openssl/usr/lib/libssl.so.1.1
#endif
#ifneq ($(ARCH),mipsel)
#	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
#endif
ifeq ($(CONFIG_FREERADIUS),y)
	$(MAKE) -C openssl build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
	-mkdir -p $(INSTALLDIR)/openssl/etc/ssl
	-touch $(INSTALLDIR)/openssl/etc/ssl/openssl.cnf
endif
	@true

openssl-clean:
	$(MAKE) -C openssl clean



OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-camellia no-whirlpool no-seed -no-gost no-ssl3 no-heartbeats no-rc2 no-weak-ssl-ciphers no-zlib no-aria no-devcryptoeng no-siphash no-sm2 no-sm3 no-sm4 no-tests no-external-tests

OPENSSL_OPTIONS:= no-err threads no-ssl2 enable-ssl3-method no-ec2m no-heartbeats no-egd no-devcryptoeng

ifeq ($(CONFIG_XSCALE),y)
OPENSSL_OPTIONS += -DHAVE_CRYPTODEV 
endif
ifeq ($(CONFIG_NEWPORT),y)
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
ifneq ($(ARCH),mips64)
ifneq ($(ARCH),x86_64)
ifneq ($(ARCH),i386)
ifneq ($(ARCH),aarch64)
ifneq ($(ARCH),arm)
OPENSSL_OPTIONS += -DOPENSSL_SMALL_FOOTPRINT
endif
endif
endif
endif
endif
ifeq ($(ARCH),mips64)
OPENSSL_OPTIONS += -O3
endif
ifeq ($(ARCH),x86_64)
OPENSSL_OPTIONS += -O3
endif
ifeq ($(ARCH),i386)
OPENSSL_OPTIONS += -O3
endif
ifeq ($(ARCH),aarch64)
OPENSSL_OPTIONS += -O3
endif
ifeq ($(ARCH),arm)
OPENSSL_OPTIONS += -O3
endif


openssl-configure:
	cd openssl && CROSS_COMPILE= && ./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--libdir=/usr/lib \
			--openssldir=/etc/ssl \
			$(COPTS) $(MIPS16_OPT) $(OPENSSL_CMAKEFLAGS) -Os -DNDEBUG -D_GNU_SOURCE \
			$(TARGET_LDFLAGS) -ldl -lrt -L$(TOP)/libucontext -lucontext \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS)
ifeq ($(ARCH),mips)
	cd openssl && patch -p0 < mips.diff
endif
ifeq ($(ARCH),mipsel)
	cd openssl && patch -p0 < mips.diff
endif
#ifeq ($(ARCH),mips64)
#	rm openssl/crypto/aes/build.info
#	svn up openssl
#	cd openssl && patch -p0 < octeon.patch
#endif

	$(MAKE) -C openssl clean
	-$(MAKE) -C openssl CC="$(CC) -I$(TOP)/openssl/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_libs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-rm -f openssl/apps/openssl
