# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2002 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LSRCFILES = README

LTCOMMAND = xfs_repair

HFILES = \
	agheader.h \
	agbtree.h \
	attr_repair.h \
	avl.h \
	bulkload.h \
	bmap.h \
	btree.h \
	da_util.h \
	dinode.h \
	dir2.h \
	err_protos.h \
	globals.h \
	incore.h \
	prefetch.h \
	progress.h \
	protos.h \
	quotacheck.h \
	rmap.h \
	rt.h \
	scan.h \
	slab.h \
	threads.h \
	versions.h

CFILES = \
	agheader.c \
	agbtree.c \
	attr_repair.c \
	avl.c \
	bulkload.c \
	bmap.c \
	btree.c \
	da_util.c \
	dino_chunks.c \
	dinode.c \
	dir2.c \
	globals.c \
	incore_bmc.c \
	incore.c \
	incore_ext.c \
	incore_ino.c \
	init.c \
	phase1.c \
	phase2.c \
	phase3.c \
	phase4.c \
	phase5.c \
	phase6.c \
	phase7.c \
	prefetch.c \
	progress.c \
	quotacheck.c \
	rmap.c \
	rt.c \
	sb.c \
	scan.c \
	slab.c \
	threads.c \
	versions.c \
	xfs_repair.c

LLDLIBS = $(LIBXFS) $(LIBXLOG) $(LIBXCMD) $(LIBFROG) $(LIBUUID) $(LIBRT) \
	$(LIBBLKID) $(LIBURCU) $(LIBPTHREAD)
LTDEPENDENCIES = $(LIBXFS) $(LIBXLOG) $(LIBXCMD) $(LIBFROG)
LLDFLAGS = -static-libtool-libs

default: depend $(LTCOMMAND)

globals.o: globals.h

include $(BUILDRULES)

#
# Tracing flags:
# -DXR_INODE_TRACE	inode processing
# -DXR_DIR_TRACE	directory processing
# -DXR_DUP_TRACE	duplicate extent processing
# -DXR_BCNT_TRACE	incore bcnt freespace btree building
# -DXR_BLD_FREE_TRACE	building on-disk freespace (bcnt/bno) btrees
# -DXR_BLD_INO_TRACE	building on-disk inode allocation btrees
# -DXR_BLD_ADD_EXTENT	track phase 5 block extent creation
# -DXR_BCKPTR_DBG	parent list debugging info
# -DXR_PF_TRACE		prefetch trace
#
#CFLAGS += ...

install: default
	$(INSTALL) -m 755 -d $(PKG_ROOT_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_ROOT_SBIN_DIR)
install-dev:

-include .dep
