export OPENSSL_TARGET := linux-generic32
ifeq ($(ARCH),armeb)
export OPENSSL_TARGET := linux-arm-openwrt
export OPENSSL_CMAKEFLAGS := -DASMAES512   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif


ifeq ($(ARCHITECTURE),ventana)
ifeq ($(ARCH),arm)
export OPENSSL_TARGET := linux-arm-openwrt
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
else
ifeq ($(ARCH),arm)
ifeq ($(CONFIG_STORM),y)
export OPENSSL_TARGET := linux-arm-openwrt
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 

else
export OPENSSL_TARGET := linux-arm-openwrt
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
endif
endif
ifeq ($(ARCH),mips)
export OPENSSL_TARGET := linux-mips-openwrt
export OPENSSL_CMAKEFLAGS := -DASMAES512 -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),mips64)
export OPENSSL_TARGET := linux-mips64-openwrt
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections -DOCTEON -DOCTEON_OPENSSL -I$(SSLPATH)/include/executive 
endif
ifeq ($(ARCH),mipsel)
export OPENSSL_TARGET := linux-mips-openwrt
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),powerpc)
export OPENSSL_TARGET := linux-generic32-openwrt
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),i386)
export OPENSSL_TARGET := linux-i386-openwrt
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),x86_64)
export OPENSSL_TARGET := linux-x86_64-openwrt
export OPENSSL_CMAKEFLAGS :=   -ffunction-sections -fdata-sections -Wl,--gc-sections 
endif
ifeq ($(ARCH),aarch64)
export OPENSSL_TARGET := linux-aarch64-openwrt
export OPENSSL_CMAKEFLAGS := -ffunction-sections -fdata-sections -Wl,--gc-sections
endif

OPENSSL_CMAKEFLAGS+= -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include

openssl:
	$(MAKE) -C openssl3 MAKE=make CC="$(CC) -I$(SSLPATH)/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE) -C openssl3 build_libs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	$(MAKE)  -C openssl3 build_programs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	rm -f openssl3/apps/openssl

openssl-shared: openssl
	$(MAKE) -C openssl3 build_libs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)


openssl-apps: openssl-shared	
	-rm openssl3/apps/openssl
	$(MAKE) -C openssl3 build_programs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-apps-static:
	-rm openssl3/libcrypto.so.1.1
	-rm openssl3/libssl.so.1.1
	-rm openssl3/apps/openssl
	$(MAKE) -C openssl3 build_programs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)

openssl-install:
#ifeq ($(CONFIG_MADWIFI),y)
	-install -D openssl3/libcrypto.so.3 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.3
	-install -D openssl3/libssl.so.3 $(INSTALLDIR)/openssl/usr/lib/libssl.so.3
#endif
#ifneq ($(ARCH),mipsel)
#	-install -D openssl3/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
#endif
ifeq ($(CONFIG_FREERADIUS),y)
	$(MAKE) -C openssl3 build_programs MAKE=make CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-install -D openssl3/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl 
endif
	-mkdir -p $(INSTALLDIR)/openssl/etc/ssl
	-touch $(INSTALLDIR)/openssl/etc/ssl/openssl.cnf

	install -d $(INSTALLDIR)/openssl/etc/ssl/modules.cnf.d $(INSTALLDIR)/openssl/usr/lib/engines-3 $(INSTALLDIR)/openssl/usr/lib/ossl-modules
	cp openssl3/apps/openssl.cnf $(INSTALLDIR)/openssl/etc/ssl/
	-cp openssl3/engines/*.so $(INSTALLDIR)/openssl/usr/lib/engines-3
#	sed 's!engines = engines_sect!#&!' $(INSTALLDIR)/openssl/etc/ssl/openssl.cnf
	cp openssl3/apps/devcrypto.cnf $(INSTALLDIR)/openssl/etc/ssl/modules.cnf.d/
	cp openssl3/apps/afalg.cnf $(INSTALLDIR)/openssl/etc/ssl/modules.cnf.d/
	cp openssl3/providers/*.so $(INSTALLDIR)/openssl/usr/lib/ossl-modules

openssl-clean:
	$(MAKE) -C openssl3 clean MAKE=make



OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-camellia no-whirlpool no-seed -no-gost no-ssl3 no-heartbeats no-rc2 no-weak-ssl-ciphers no-zlib no-aria no-siphash no-sm2 no-sm3 no-sm4 no-tests no-external-tests no-mdc2 no-aria no-sm2 no-sm3 no-sm4 no-err

OPENSSL_OPTIONS:= no-err threads no-ssl2 enable-ssl3-method no-ec2m no-heartbeats no-egd no-nextprotoneg no-brotli no-argon2 no-winstore no-docs no-ecx no-quic no-unstable-qlog no-sm2-precomp no-zstd

ifeq ($(CONFIG_IPQ806X),y)
OPENSSL_OPTIONS += enable-devcryptoeng enable-afalgeng
endif
ifeq ($(CONFIG_IPQ6018),y)
OPENSSL_OPTIONS += enable-devcryptoeng enable-afalgeng
endif
#ifeq ($(CONFIG_ALPINE),y)
#OPENSSL_OPTIONS += enable-devcryptoeng
#endif
ifeq ($(CONFIG_MVEBU),y)
OPENSSL_OPTIONS += enable-devcryptoeng
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
	cd openssl3 && CROSS_COMPILE= MAKE=make && ./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--libdir=/usr/lib \
			--openssldir=/etc/ssl \
			$(COPTS) $(MIPS16_OPT) $(OPENSSL_CMAKEFLAGS) -Os -DNDEBUG \
			$(TARGET_LDFLAGS) -ldl -lrt -L$(TOP)/libucontext -lucontext \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS)
ifeq ($(ARCH),mips)
	-cd openssl3 && patch -p0 < mips.diff
endif
ifeq ($(ARCH),mipsel)
	-cd openssl3 && patch -p0 < mips.diff
endif
#ifeq ($(ARCH),mips64)
#	rm openssl3/crypto/aes/build.info
#	svn up openssl3
#	cd openssl3 && patch -p0 < octeon.patch
#endif

	$(MAKE) -C openssl3 MAKE=make clean
	-$(MAKE) -C openssl3 MAKE=make CC="$(CC) -I$(SSLPATH)/crypto -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl3 MAKE=make build_libs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-$(MAKE) -C openssl3 MAKE=make build_programs CC="$(CC) -fPIC" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc $(OPENSSL_MAKEFLAGS)
	-rm -f openssl3/apps/openssl
