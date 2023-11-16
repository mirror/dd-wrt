#CFLAGS += -DALL_DH_GROUPS
CFLAGS += -DCONFIG_SHA256
CFLAGS += -DCONFIG_SHA384
CFLAGS += -DCONFIG_HMAC_SHA256_KDF
CFLAGS += -DCONFIG_HMAC_SHA384_KDF

# crypto_module_tests.c
CFLAGS += -DCONFIG_MODULE_TESTS
CFLAGS += -DCONFIG_DPP
#CFLAGS += -DCONFIG_DPP2
#CFLAGS += -DCONFIG_DPP3
CFLAGS += -DCONFIG_ECC
CFLAGS += -DCONFIG_MESH
CFLAGS += -DEAP_PSK
CFLAGS += -DEAP_FAST

ifeq ($(CONFIG_TLS),mbedtls)

# (enable features for 'cd tests; make run-tests CONFIG_TLS=mbedtls')
CFLAGS += -DCRYPTO_RSA_OAEP_SHA256
CFLAGS += -DCONFIG_DES
CFLAGS += -DEAP_IKEV2
CFLAGS += -DEAP_MSCHAPv2
CFLAGS += -DEAP_SIM

LIB_OBJS = tls_mbedtls.o crypto_mbedtls.o
LIB_OBJS+= \
	aes-eax.o \
	aes-siv.o \
	dh_groups.o \
	milenage.o \
	ms_funcs.o

else
ifeq ($(CONFIG_TLS),openssl)

# (enable features for 'cd tests; make run-tests CONFIG_TLS=openssl')
ifndef CONFIG_TLS_DEFAULT_CIPHERS
CONFIG_TLS_DEFAULT_CIPHERS = "DEFAULT:!EXP:!LOW"
endif
CFLAGS += -DTLS_DEFAULT_CIPHERS=\"$(CONFIG_TLS_DEFAULT_CIPHERS)\"
CFLAGS += -DCRYPTO_RSA_OAEP_SHA256
CFLAGS += -DEAP_TLS_OPENSSL

LIB_OBJS = tls_openssl.o fips_prf_openssl.o crypto_openssl.o
LIB_OBJS+= \
	aes-ctr.o \
	aes-eax.o \
	aes-encblock.o \
	aes-siv.o \
	dh_groups.o \
	milenage.o \
	ms_funcs.o \
	sha1-prf.o \
	sha1-tlsprf.o \
	sha1-tprf.o \
	sha256-kdf.o \
	sha256-prf.o \
	sha256-tlsprf.o

else
ifeq ($(CONFIG_TLS),wolfssl)

# (wolfssl libraries must be built with ./configure --enable-wpas)
# (enable features for 'cd tests; make run-tests CONFIG_TLS=wolfssl')
CFLAGS += -DWOLFSSL_DER_LOAD
CFLAGS += -DCONFIG_DES

LIB_OBJS = tls_wolfssl.o fips_prf_wolfssl.o crypto_wolfssl.o
LIB_OBJS+= \
	aes-ctr.o \
	aes-eax.o \
	aes-encblock.o \
	aes-siv.o \
	dh_groups.o \
	milenage.o \
	ms_funcs.o \
	sha1-prf.o \
	sha1-tlsprf.o \
	sha1-tprf.o \
	sha256-kdf.o \
	sha256-prf.o \
	sha256-tlsprf.o

else
ifeq ($(CONFIG_TLS),gnutls)

# (enable features for 'cd tests; make run-tests CONFIG_TLS=gnutls')
LIB_OBJS = tls_gnutls.o crypto_gnutls.o
LIB_OBJS+= \
	aes-cbc.o \
	aes-ctr.o \
	aes-eax.o \
	aes-encblock.o \
	aes-omac1.o \
	aes-siv.o \
	aes-unwrap.o \
	aes-wrap.o \
	dh_group5.o \
	dh_groups.o \
	milenage.o \
	ms_funcs.o \
	rc4.o \
	sha1-pbkdf2.o \
	sha1-prf.o \
	fips_prf_internal.o \
	sha1-internal.o \
	sha1-tlsprf.o \
	sha1-tprf.o \
	sha256-kdf.o \
	sha256-prf.o \
	sha256-tlsprf.o

else

CFLAGS += -DCONFIG_CRYPTO_INTERNAL
CFLAGS += -DCONFIG_TLS_INTERNAL_CLIENT
CFLAGS += -DCONFIG_TLS_INTERNAL_SERVER
CFLAGS += -DCONFIG_INTERNAL_SHA384

LIB_OBJS= \
	aes-cbc.o \
	aes-ccm.o \
	aes-ctr.o \
	aes-eax.o \
	aes-encblock.o \
	aes-internal.o \
	aes-internal-dec.o \
	aes-internal-enc.o \
	aes-omac1.o \
	aes-siv.o \
	aes-unwrap.o \
	aes-wrap.o \
	des-internal.o \
	dh_group5.o \
	dh_groups.o \
	md4-internal.o \
	md5.o \
	md5-internal.o \
	milenage.o \
	ms_funcs.o \
	rc4.o \
	sha1.o \
	sha1-internal.o \
	sha1-pbkdf2.o \
	sha1-prf.o \
	sha1-tlsprf.o \
	sha1-tprf.o \
	sha256.o \
	sha256-kdf.o \
	sha256-prf.o \
	sha256-tlsprf.o \
	sha256-internal.o \
	sha384.o \
	sha384-prf.o \
	sha384-internal.o \
	sha512.o \
	sha512-prf.o \
	sha512-internal.o

LIB_OBJS += crypto_internal.o
LIB_OBJS += crypto_internal-cipher.o
LIB_OBJS += crypto_internal-modexp.o
LIB_OBJS += crypto_internal-rsa.o
LIB_OBJS += tls_internal.o
LIB_OBJS += fips_prf_internal.o

endif
endif
endif
endif


# (used by wlantest/{bip,gcmp,rx_mgmt}.c and tests/test-aes.c)
LIB_OBJS += aes-gcm.o

ifndef TEST_FUZZ
LIB_OBJS += random.o
endif

include ../lib.rules
