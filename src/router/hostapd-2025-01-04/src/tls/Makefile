LIB_OBJS= asn1.o

ifneq ($(CONFIG_TLS),gnutls)
ifneq ($(CONFIG_TLS),mbedtls)
ifneq ($(CONFIG_TLS),openssl)
ifneq ($(CONFIG_TLS),wolfssl)

CFLAGS += -DCONFIG_INTERNAL_LIBTOMMATH
CFLAGS += -DCONFIG_CRYPTO_INTERNAL
CFLAGS += -DCONFIG_TLSV11
CFLAGS += -DCONFIG_TLSV12

LIB_OBJS= \
	asn1.o \
	bignum.o \
	pkcs1.o \
	pkcs5.o \
	pkcs8.o \
	rsa.o \
	tlsv1_client.o \
	tlsv1_client_read.o \
	tlsv1_client_write.o \
	tlsv1_client_ocsp.o \
	tlsv1_common.o \
	tlsv1_cred.o \
	tlsv1_record.o \
	tlsv1_server.o \
	tlsv1_server_read.o \
	tlsv1_server_write.o \
	x509v3.o
endif
endif
endif
endif

include ../lib.rules
