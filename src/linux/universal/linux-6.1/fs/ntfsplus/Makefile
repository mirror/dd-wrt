# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the ntfsplus filesystem support.
#

# to check robot warnings
ccflags-y += -Wint-to-pointer-cast \
        $(call cc-option,-Wunused-but-set-variable,-Wunused-const-variable) \
        $(call cc-option,-Wold-style-declaration,-Wout-of-line-declaration)

obj-$(CONFIG_NTFSPLUS_FS) += ntfsplus.o

ntfsplus-y := aops.o attrib.o collate.o misc.o dir.o file.o index.o inode.o \
	  mft.o mst.o namei.o runlist.o super.o unistr.o attrlist.o ea.o \
	  upcase.o bitmap.o lcnalloc.o logfile.o reparse.o compress.o \
	  ntfs_iomap.o

ccflags-$(CONFIG_NTFSPLUS_DEBUG) += -DDEBUG
