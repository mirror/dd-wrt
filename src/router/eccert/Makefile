# eccert - minimal P-256 key/CSR/cert tool
#   make BACKEND=p256m   p256-m: constant-time, smaller (default if p256-m/ is present)
#   make BACKEND=uecc    micro-ecc
#   make test            OpenSSL regression test (needs openssl)
#   make rfc6979         RFC 6979 vectors + cross-check (needs python3 + pip "ecdsa")
CC       ?= gcc
CFLAGS   ?= -Os
BACKEND  ?= $(if $(wildcard p256-m/p256-m.c),p256m,uecc)
OBJCOPY  ?= objcopy

ifeq ($(BACKEND),uecc)
UECC     ?= micro-ecc
UECC_OPT ?= 1
EC_SRC    = $(UECC)/uECC.c
EC_FLAGS  = -I$(UECC) -DuECC_SUPPORTS_secp160r1=0 -DuECC_SUPPORTS_secp192r1=0 \
            -DuECC_SUPPORTS_secp224r1=0 -DuECC_SUPPORTS_secp256k1=0 \
            -DuECC_SUPPORT_COMPRESSED_POINT=0 -DuECC_OPTIMIZATION_LEVEL=$(UECC_OPT) \
            -DuECC_PLATFORM=uECC_arch_other
else
EC_SRC    = p256-m/p256-m.c
# p256-m uses 16x16->32 multiplies by default (constant-time even on cores with
# early-out multipliers). Add -DMUL64_IS_CONSTANT_TIME only if your CPU's 32x32->64
# multiply is known to be constant-time.
EC_FLAGS  = -DECCERT_P256M -DMBEDTLS_PSA_P256M_DRIVER_ENABLED -Ip256-m -Ishim
endif

ALL_CFLAGS  = $(CFLAGS) $(LTO) $(EC_FLAGS) -ffunction-sections -fdata-sections \
              -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -Wall
ALL_LDFLAGS = $(LDFLAGS) -Wl,--gc-sections -Wl,--build-id=none -s

eccert: eccert.c $(EC_SRC)
	$(CC) $(ALL_CFLAGS) -o $@ eccert.c $(EC_SRC) $(ALL_LDFLAGS)
	-$(OBJCOPY) -R .pdr -R .comment -R .mdebug.abi32 -R .note.gnu.build-id $@

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp -f eccert $(INSTALLDIR)/usr/sbin

test: eccert
	tests/check.sh ./eccert

rfc6979:
	$(CC) -O2 -w $(EC_FLAGS) -o tests/selftest tests/selftest.c $(EC_SRC)
	python3 tests/rfc6979.py tests/selftest

clean:
	rm -f eccert tests/selftest

.PHONY: test rfc6979 clean
