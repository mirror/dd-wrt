#
# Makefile for the Linux msdos filesystem routines.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile.


EXTRA_CFLAGS += -DCONFIG_YAFFS_YAFFS1 -DCONFIG_YAFFS_YAFFS2


obj-$(CONFIG_YAFFS_FS) += yaffs.o

yaffs-y = yaffs_fs.o yaffs_guts.o yaffs_mtdif.o yaffs_tagscompat.o \
        yaffs_packedtags2.o yaffs_mtdif2.o yaffs_tagsvalidity.o \
	yaffs_ecc.o
