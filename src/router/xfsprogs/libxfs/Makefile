#
# Copyright (c) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxfs.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0

# headers to install in include/xfs
PKGHFILES = xfs_fs.h \
	xfs_types.h \
	xfs_da_format.h \
	xfs_format.h \
	xfs_log_format.h

HFILES = \
	xfs_ag_resv.h \
	xfs_alloc.h \
	xfs_alloc_btree.h \
	xfs_attr_leaf.h \
	xfs_attr_sf.h \
	xfs_bit.h \
	xfs_bmap.h \
	xfs_bmap_btree.h \
	xfs_btree.h \
	xfs_attr_remote.h \
	xfs_cksum.h \
	xfs_da_btree.h \
	xfs_dir2.h \
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
	libxfs_io.h \
	libxfs_api_defs.h \
	init.h \
	crc32defs.h \
	crc32table.h \
	libxfs_priv.h \
	xfs_dir2_priv.h

CFILES = cache.c \
	crc32.c \
	defer_item.c \
	init.c \
	kmem.c \
	list_sort.c \
	logitem.c \
	radix-tree.c \
	rdwr.c \
	trans.c \
	util.c \
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
	xfs_da_btree.c \
	xfs_da_format.c \
	xfs_defer.c \
	xfs_dir2.c \
	xfs_dir2_block.c \
	xfs_dir2_data.c \
	xfs_dir2_leaf.c \
	xfs_dir2_node.c \
	xfs_dir2_sf.c \
	xfs_dquot_buf.c \
	xfs_ialloc.c \
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
	xfs_trans_resv.c

CFILES += $(PKG_PLATFORM).c
PCFILES = darwin.c freebsd.c irix.c linux.c
LSRCFILES = $(shell echo $(PCFILES) | sed -e "s/$(PKG_PLATFORM).c//g")
LSRCFILES += gen_crc32table.c

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

LDIRT = gen_crc32table crc32table.h crc32selftest

default: crc32selftest ltdepend $(LTLIBRARY)

crc32table.h: gen_crc32table.c
	@echo "    [CC]     gen_crc32table"
	$(Q) $(BUILD_CC) $(CFLAGS) -o gen_crc32table $<
	@echo "    [GENERATE] $@"
	$(Q) ./gen_crc32table > crc32table.h

# The selftest binary will return an error if it fails. This is made a
# dependency of the build process so that we refuse to build the tools on broken
# systems/architectures. Hence we make sure that xfsprogs will never use a
# busted CRC calculation at build time and hence avoid putting bad CRCs down on
# disk.
crc32selftest: gen_crc32table.c crc32table.h crc32.c
	@echo "    [TEST]    CRC32"
	$(Q) $(BUILD_CC) $(CFLAGS) -D CRC32_SELFTEST=1 crc32.c -o $@
	$(Q) ./$@

# set up include/xfs header directory
include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_INC_DIR)

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
