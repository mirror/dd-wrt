#
# Makefile for the Linux kernel crypto library.
#
# 1 Dec 2001, Herbert Valerio Riedel <hvr@kernel.org>
#

mod-subdirs := ciphers digests drivers

O_TARGET := crypto.o

export-objs := cryptoapi.o

obj-$(CONFIG_CRYPTO)           += cryptoapi.o

subdir-$(CONFIG_CIPHERS)       += ciphers
subdir-$(CONFIG_DIGESTS)       += digests
subdir-$(CONFIG_CRYPTODEV)     += drivers

obj-y += $(join $(subdir-y),$(subdir-y:%=/%.o))

include $(TOPDIR)/Rules.make
