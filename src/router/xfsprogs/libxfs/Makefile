# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxfs.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0
# we need a static build even if --disable-static is specified
#LTLDFLAGS += -static

# headers to install in include/xfs
PKGHFILES = xfs_fs.h \
	xfs_types.h \
	xfs_da_format.h \
	xfs_format.h \
	xfs_log_format.h

HFILES = \
	libxfs_io.h \
	libxfs_api_defs.h \
	init.h \
	libxfs_priv.h \
	linux-err.h \
	topology.h \
	xfs_ag_resv.h \
	xfs_alloc.h \
	xfs_alloc_btree.h \
	xfs_attr_leaf.h \
	xfs_attr_sf.h \
	xfs_bit.h \
	xfs_bmap.h \
	xfs_bmap_btree.h \
	xfs_btree.h \
	xfs_btree_staging.h \
	xfs_attr_remote.h \
	xfs_cksum.h \
	xfs_da_btree.h \
	xfs_dir2.h \
	xfs_errortag.h \
	xfs_ialloc.h \
	xfs_ialloc_btree.h \
	xfs_inode_buf.h \
	xfs_inode_fork.h \
	xfs_quota_defs.h \
	xfs_refcount.h \
	xfs_refcount_btree.h \
	xfs_rmap.h \
	xfs_rmap_btree.h \
	xfs_sb.h \
	xfs_shared.h \
	xfs_trans_resv.h \
	xfs_trans_space.h \
	xfs_dir2_priv.h

CFILES = cache.c \
	defer_item.c \
	init.c \
	kmem.c \
	logitem.c \
	rdwr.c \
	topology.c \
	trans.c \
	util.c \
	xfs_ag.c \
	xfs_ag_resv.c \
	xfs_alloc.c \
	xfs_alloc_btree.c \
	xfs_attr.c \
	xfs_attr_leaf.c \
	xfs_attr_remote.c \
	xfs_bit.c \
	xfs_bmap.c \
	xfs_bmap_btree.c \
	xfs_btree.c \
	xfs_btree_staging.c \
	xfs_da_btree.c \
	xfs_defer.c \
	xfs_dir2.c \
	xfs_dir2_block.c \
	xfs_dir2_data.c \
	xfs_dir2_leaf.c \
	xfs_dir2_node.c \
	xfs_dir2_sf.c \
	xfs_dquot_buf.c \
	xfs_ialloc.c \
	xfs_iext_tree.c \
	xfs_inode_buf.c \
	xfs_inode_fork.c \
	xfs_ialloc_btree.c \
	xfs_log_rlimit.c \
	xfs_refcount.c \
	xfs_refcount_btree.c \
	xfs_rmap.c \
	xfs_rmap_btree.c \
	xfs_rtbitmap.c \
	xfs_sb.c \
	xfs_symlink_remote.c \
	xfs_trans_inode.c \
	xfs_trans_resv.c \
	xfs_types.c

#
# Tracing flags:
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

# set up include/xfs header directory
include $(BUILDRULES)

install: default
	$(INSTALL_LTLIB)
#	$(INSTALL) -m 755 -d $(PKG_INC_DIR)

install-headers: $(addsuffix -hdrs, $(PKGHFILES))

%-hdrs:
	$(Q)$(LN_S) -f $(CURDIR)/$* $(TOPDIR)/include/xfs/$*

install-dev: install
	$(INSTALL) -m 644 $(PKGHFILES) $(PKG_INC_DIR)

# We need to install the headers before building the dependencies.  If we
# include the .ltdep file, the makefile decides that it needs to build the
# dependencies to remake the makefile before running the header install target,
# hence making it impossible to avoid errors being thrown by the dependency
# generation. Hence we play games so that we only include this file if we aren't
# running the install-headers target.
ifndef NODEP
-include .ltdep
endif
