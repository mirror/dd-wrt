#
# Copyright (c) 2000-2002 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LSRCFILES = README

LTCOMMAND = xfs_repair

HFILES = agheader.h attr_repair.h avl.h avl64.h bmap.h btree.h \
	dinode.h dir.h dir2.h err_protos.h globals.h incore.h protos.h rt.h \
	progress.h scan.h versions.h prefetch.h threads.h

CFILES = agheader.c attr_repair.c avl.c avl64.c bmap.c btree.c \
	dino_chunks.c dinode.c dir.c dir2.c globals.c incore.c \
	incore_bmc.c init.c incore_ext.c incore_ino.c phase1.c \
	phase2.c phase3.c phase4.c phase5.c phase6.c phase7.c \
	progress.c prefetch.c rt.c sb.c scan.c threads.c \
	versions.c xfs_repair.c

LLDLIBS = $(LIBXFS) $(LIBXLOG) $(LIBUUID) $(LIBRT) $(LIBPTHREAD)
LTDEPENDENCIES = $(LIBXFS) $(LIBXLOG)
LLDFLAGS = -static

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
