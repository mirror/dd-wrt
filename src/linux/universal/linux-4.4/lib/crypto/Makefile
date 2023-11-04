# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_CRYPTO_LIB_UTILS)			+= libcryptoutils.o
libcryptoutils-y				:= memneq.o utils.o

obj-$(CONFIG_CRYPTO_LIB_POLY1305_GENERIC)	+= libpoly1305.o
libpoly1305-y					:= poly1305-donna32.o
libpoly1305-$(CONFIG_ARCH_SUPPORTS_INT128)	:= poly1305-donna64.o
libpoly1305-y					+= poly1305.o

