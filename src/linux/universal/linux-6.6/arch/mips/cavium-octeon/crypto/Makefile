# SPDX-License-Identifier: GPL-2.0
#
# OCTEON-specific crypto modules.
#
ccflags-y := -DCVMX_BUILD_FOR_LINUX_KERNEL -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DOCTEON_MODEL=OCTEON_CN70XX -DCVMX_ENABLE_PARAMETER_CHECKING=0

obj-y += octeon-crypto.o

obj-$(CONFIG_CRYPTO_MD5_OCTEON)		+= octeon-md5.o
obj-$(CONFIG_CRYPTO_SHA1_OCTEON)	+= octeon-sha1.o
obj-$(CONFIG_CRYPTO_SHA256_OCTEON)	+= octeon-sha256.o
obj-$(CONFIG_CRYPTO_SHA512_OCTEON)	+= octeon-sha512.o
