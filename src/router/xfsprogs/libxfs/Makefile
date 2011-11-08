#
# Copyright (c) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxfs.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0

HFILES = xfs.h init.h
CFILES = cache.c init.c kmem.c logitem.c radix-tree.c rdwr.c trans.c util.c \
	xfs_alloc.c xfs_ialloc.c xfs_inode.c xfs_btree.c xfs_alloc_btree.c \
	xfs_ialloc_btree.c xfs_bmap_btree.c xfs_da_btree.c \
	xfs_dir2.c xfs_dir2_leaf.c xfs_attr_leaf.c xfs_dir2_block.c \
	xfs_dir2_node.c xfs_dir2_data.c xfs_dir2_sf.c xfs_bmap.c \
	xfs_mount.c xfs_rtalloc.c xfs_trans.c xfs_attr.c

CFILES += $(PKG_PLATFORM).c
PCFILES = darwin.c freebsd.c irix.c linux.c
LSRCFILES = $(shell echo $(PCFILES) | sed -e "s/$(PKG_PLATFORM).c//g")

#
# Tracing flags:
# -DIO_DEBUG		reads and writes of buffers
# -DMEM_DEBUG		all zone memory use
# -DLI_DEBUG		log item (ino/buf) manipulation
# -DXACT_DEBUG		transaction state changes
#
#LCFLAGS +=

FCFLAGS = -I.

LTLIBS = $(LIBPTHREAD) $(LIBRT)

# don't try linking xfs_repair with a debug libxfs.
DEBUG = -DNDEBUG

default: ltdepend $(LTLIBRARY)

include $(BUILDRULES)

install: default
	$(INSTALL_LTLIB)

install-dev: default

install-qa: default

-include .ltdep
