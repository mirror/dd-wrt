#
# Makefile for the linux squashfs routines.
#

obj-$(CONFIG_SQUASHFS) += squashfs.o
squashfs-$(CONFIG_SQUASHFS) += inode.o
squashfs-$(CONFIG_SQUASHFS) += LzmaDecode.o
EXTRA_CFLAGS += -Iinclude/linux -Iinclude/linux/lzma -include types.h
