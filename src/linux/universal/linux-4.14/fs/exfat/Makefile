#
# Makefile for Linux FAT12/FAT16/FAT32(VFAT)/FAT64(ExFAT) filesystem driver.
#

# call from kernel build system
obj-$(CONFIG_EXFAT_FS) += exfat.o

exfat-objs	:= super.o core.o core_exfat.o blkdev.o fatent.o cache.o \
		   nls.o misc.o extent.o xattr.o


.PHONY : all clean install uninstall
