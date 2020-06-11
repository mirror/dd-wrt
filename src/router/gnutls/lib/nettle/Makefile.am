## Process this file with automake to produce Makefile.in
# Copyright (C) 2004-2012 Free Software Foundation, Inc.
#
# Author: Nikos Mavroyanopoulos
#
# This file is part of GNUTLS.
#
# The GNUTLS library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
#
# The GNUTLS library is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>

include $(top_srcdir)/lib/common.mk

AM_CFLAGS += $(HOGWEED_CFLAGS) $(GMP_CFLAGS)

AM_CPPFLAGS = \
	-I$(srcdir)/int		\
	-I$(srcdir)/backport		\
	-I$(srcdir)/../../gl		\
	-I$(builddir)/../../gl		\
	-I$(srcdir)/../includes		\
	-I$(builddir)/../includes	\
	-I$(builddir)/../../gl          \
	-I$(srcdir)/..

if ENABLE_MINITASN1
AM_CPPFLAGS += -I$(srcdir)/../minitasn1
endif

noinst_LTLIBRARIES = libcrypto.la

libcrypto_la_SOURCES = pk.c mpi.c mac.c cipher.c init.c \
	gnettle.h rnd-common.h prf.c \
	rnd.c int/rsa-fips.h int/rsa-keygen-fips186.c int/provable-prime.c \
	int/dsa-fips.h int/dsa-keygen-fips186.c int/dsa-validate.c \
	int/tls1-prf.c int/tls1-prf.h \
	int/dsa-compute-k.c int/dsa-compute-k.h \
	int/ecdsa-compute-k.c int/ecdsa-compute-k.h \
	int/mpn-base256.c int/mpn-base256.h \
	int/block8.h backport/block-internal.h

if WINDOWS
if HAVE_BCRYPT
libcrypto_la_SOURCES += sysrng-bcrypt.c
else
libcrypto_la_SOURCES += sysrng-windows.c
endif
else
if HAVE_GETENTROPY
libcrypto_la_SOURCES += sysrng-getentropy.c
else
if HAVE_KERN_ARND
libcrypto_la_SOURCES += sysrng-netbsd.c
else
libcrypto_la_SOURCES += sysrng-linux.c
endif
endif
endif

libcrypto_la_SOURCES += rnd-fuzzer.c

if ENABLE_FIPS140
libcrypto_la_SOURCES += rnd-fips.c int/drbg-aes-self-test.c \
	int/drbg-aes.c int/drbg-aes.h
endif

if ENABLE_GOST
libcrypto_la_SOURCES += \
	gost/nettle-write.h gost/write-le32.c \
	gost/gost28147.c gost/gost28147.h gost/gost-wrap.c \
	gost/gosthash94.c gost/gosthash94.h gost/gosthash94-meta.c \
	gost/streebog.c gost/streebog.h gost/streebog-meta.c \
	gost/hmac-gosthash94.c gost/hmac-streebog.c gost/hmac-gost.h

libcrypto_la_SOURCES += \
	gost/bignum-le.c gost/bignum-le.h

libcrypto_la_SOURCES += \
	gost/gostdsa-mask.c gost/gostdsa2.h

libcrypto_la_SOURCES += gost_keywrap.c
endif

if NEED_INT_ECC
ecc_generated_headers = \
	ecc/ecc-gost-gc256b-32.h ecc/ecc-gost-gc256b-64.h \
	ecc/ecc-gost-gc512a-32.h ecc/ecc-gost-gc512a-64.h \
	ecc/ecc-curve448-32.h ecc/ecc-curve448-64.h

BUILT_SOURCES = $(ecc_generated_headers)
EXTRA_DIST = $(ecc_generated_headers) ecc/eccdata.stamp

noinst_PROGRAMS = ecc/eccdata$(EXEEXT)

ecc_eccdata_SOURCES = ecc/eccdata.c
ecc_eccdata_CFLAGS = $(GMP_CFLAGS)
ecc_eccdata_LDADD = $(GMP_LIBS) ../../gl/libgnu.la

ecc/eccdata.stamp: $(ecc_eccdata_SOURCES)
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) ecc/eccdata$(EXEEXT) && touch $@

ecc/ecc-curve448-32.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) curve448 38 6 32 > $@T && mv $@T $@

ecc/ecc-curve448-64.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) curve448 38 6 64 > $@T && mv $@T $@

ecc/ecc-gost-gc256b-32.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) gost_gc256b 11 6 32 > $@T && mv $@T $@

ecc/ecc-gost-gc256b-64.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) gost_gc256b 11 6 64 > $@T && mv $@T $@

ecc/ecc-gost-gc512a-32.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) gost_gc512a 43 6 32 > $@T && mv $@T $@

ecc/ecc-gost-gc512a-64.h: ecc/eccdata.stamp
	$(AM_V_GEN)ecc/eccdata$(EXEEXT) gost_gc512a 43 6 64 > $@T && mv $@T $@

libcrypto_la_SOURCES += \
	gost/ecc-gost-hash.c nettle-alloca.h ecc-gost-curve.h

libcrypto_la_SOURCES += \
	ecc/write-le64.c \
	ecc/nettle-write.h ecc/gmp-glue.h ecc/gmp-glue.c

libcrypto_la_SOURCES += \
	ecc/sha3.c ecc/sha3.h ecc/sha3-256.c \
	ecc/sha3-permute.c ecc/sha3-internal.h \
	ecc/shake256.c

libcrypto_la_SOURCES += \
	ecc/ecc-internal.h \
	ecc/ecc-add-eh.c ecc/ecc-add-ehh.c ecc/ecc-dup-eh.c \
	ecc/ecc-eh-to-a.c ecc/ecc-mul-a-eh.c ecc/ecc-mul-g-eh.c \
	ecc/ecc-mul-m.c ecc/ecc-mod.c ecc/ecc-mod-arith.c \
	ecc/ecc-mod-inv.c \
	ecc/ecc-a-to-j.c \
	ecc/sec-tabselect.c ecc/cnd-copy.c ecc/sec-add-1.c \
	ecc/ecc-curve448.c $(ecc_genereated_headers) \
	ecc/curve448-eh-to-x.c ecc/curve448.h ecc/curve448-mul.c \
	ecc/curve448-mul-g.c

libcrypto_la_SOURCES += \
	gost/gost-point.c gost/gost-point-mul-g.c gost/gostdsa-keygen.c \
	ecc/ecc-add-jja.c ecc/ecc-add-jjj.c ecc/ecc-dup-jj.c \
	ecc/ecc-mul-a.c ecc/ecc-mul-g.c \
	ecc/ecc-random.c \
	ecc/ecc-j-to-a.c \
	ecc/ecc-gost-gc256b.c ecc/ecc-gost-gc512a.c \
	ecc/ecc-gostdsa-sign.c ecc/ecc-gostdsa-verify.c \
	ecc/gostdsa.h ecc/gostdsa-sign.c ecc/gostdsa-verify.c ecc/gostdsa-vko.c

libcrypto_la_SOURCES += \
	ecc/eddsa.h ecc/eddsa-compress.c ecc/eddsa-decompress.c \
	ecc/eddsa-expand.c ecc/eddsa-hash.c ecc/eddsa-internal.h \
	ecc/eddsa-pubkey.c ecc/eddsa-sign.c ecc/eddsa-verify.c \
	ecc/ed448-shake256.c ecc/ed448-shake256-pubkey.c \
	ecc/ed448-shake256-sign.c ecc/ed448-shake256-verify.c
endif

if NEED_CFB
libcrypto_la_SOURCES += \
	backport/cfb.c backport/cfb.h
endif

if NEED_CMAC
libcrypto_la_SOURCES += \
	backport/cmac.c backport/cmac.h \
	backport/cmac-aes128.c backport/cmac-aes256.c
endif

if NEED_XTS
libcrypto_la_SOURCES += \
	backport/xts.c backport/xts.h \
	backport/xts-aes128.c backport/xts-aes256.c
endif

if NEED_CHACHA
libcrypto_la_SOURCES += \
	backport/chacha-core-internal.c backport/chacha-crypt.c \
	backport/chacha-internal.h backport/chacha-poly1305.c \
	backport/chacha-poly1305.h backport/chacha-set-key.c \
	backport/chacha-set-nonce.c backport/chacha.h \
	backport/poly1305.h backport/poly1305-internal.c \
	backport/poly1305-internal.h
endif

if NEED_SIV
libcrypto_la_SOURCES += \
	backport/siv-cmac-aes128.c backport/siv-cmac-aes256.c \
	backport/siv-cmac.c backport/siv-cmac.h
endif
