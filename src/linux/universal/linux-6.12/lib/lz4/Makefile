# SPDX-License-Identifier: GPL-2.0-only
ccflags-y += -O3

obj-$(CONFIG_LZ4_COMPRESS) += lz4_compress.o
obj-$(CONFIG_LZ4HC_COMPRESS) += lz4hc_compress.o
obj-$(CONFIG_LZ4_DECOMPRESS) += lz4_decompress.o

obj-$(CONFIG_ARM64) += $(addprefix lz4armv8/, lz4accel.o lz4armv8.o)
