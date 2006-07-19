#
# Makefile for the linux squashfs routines.
#

obj-$(CONFIG_SQUASHFS) += squashfs.o

#squashfs-objs := inode.o

# CONFIG_MIPS_BRCM Begin Broadcom changed code.

ifdef CONFIG_ZLIB_FS_INFLATE
squashfs-objs := inode.o
else
ifdef CONFIG_LZMA_FS_INFLATE
squashfs-objs := inode.o ../inflate_lzma/7zlzma.o ../inflate_lzma/LZMADecoder.o ../inflate_lzma/IInOutStreams.o
endif
endif

# CONFIG_MIPS_BRCM End Broadcom changed code.

