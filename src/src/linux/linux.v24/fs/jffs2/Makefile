#
# Makefile for the linux Journalling Flash FileSystem (JFFS) routines.
#
# $Id: Makefile,v 1.25.2.1 2002/10/11 09:04:44 dwmw2 Exp $
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...


JFFS2_BBC_KERNEL_OBJS-y = jffs2_bbc_framework.o jffs2_bbc_fs.o

JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_ARMLIB) += jffs2_bbc_armlib_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZO) += jffs2_bbc_lzo_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZSS) += jffs2_bbc_lzss_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZARI) += jffs2_bbc_lzari_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZHD) += jffs2_bbc_lzhd_comp.o

JFFS2_BBC_KERNEL_OBJS := $(JFFS2_BBC_KERNEL_OBJS-y)

JFFS2_BBC_MKFS_OBJS   = jffs2_bbc_mkfs.o jffs2_bbc_framework.o jffs2_bbc_armlib_comp.o jffs2_bbc_lzo_comp.o\
                        jffs2_bbc_lzss_comp.o jffs2_bbc_lzari_comp.o jffs2_bbc_lzhd_comp.o

COMPR_OBJS	:= compr.o compr_rubin.o compr_rtime.o pushpull.o \
			compr_zlib.o
JFFS2_OBJS	:= dir.o file.o ioctl.o nodelist.o malloc.o \
	$(JFFS2_BBC_KERNEL_OBJS) \
	read.o nodemgmt.o readinode.o super.o write.o scan.o gc.o \
	symlink.o build.o erase.o background.o

O_TARGET := jffs2.o

obj-y := $(COMPR_OBJS) $(JFFS2_OBJS)
obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make

