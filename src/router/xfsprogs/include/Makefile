#
# Copyright (c) 2000-2006 Silicon Graphics, Inc.
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it would be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

QAHFILES = libxfs.h libxlog.h \
	atomic.h bitops.h cache.h kmem.h list.h hlist.h parent.h radix-tree.h \
	swab.h \
	xfs_ag.h xfs_alloc.h xfs_alloc_btree.h xfs_arch.h xfs_attr_leaf.h \
	xfs_attr_sf.h xfs_bit.h xfs_bmap.h xfs_bmap_btree.h xfs_btree.h \
	xfs_btree_trace.h xfs_buf_item.h xfs_da_btree.h xfs_dinode.h \
	xfs_dir2.h xfs_dir2_block.h xfs_dir2_data.h xfs_dir2_leaf.h \
	xfs_dir2_node.h xfs_dir2_sf.h xfs_dir_leaf.h xfs_dir_sf.h \
	xfs_extfree_item.h xfs_ialloc.h xfs_ialloc_btree.h \
	xfs_inode.h xfs_inode_item.h xfs_inum.h \
	xfs_log.h xfs_log_priv.h xfs_log_recover.h xfs_metadump.h \
	xfs_mount.h xfs_quota.h xfs_rtalloc.h xfs_sb.h xfs_trace.h \
	xfs_trans.h xfs_trans_space.h xfs_types.h xfs_dfrag.h

HFILES = handle.h jdm.h xqm.h xfs.h xfs_fs.h
HFILES += $(PKG_PLATFORM).h
PHFILES = darwin.h freebsd.h irix.h linux.h gnukfreebsd.h
DKHFILES = volume.h fstyp.h dvh.h
LSRCFILES = $(shell echo $(PHFILES) | sed -e "s/$(PKG_PLATFORM).h//g")
LSRCFILES += platform_defs.h.in builddefs.in buildmacros buildrules install-sh
LSRCFILES += $(DKHFILES) command.h input.h path.h project.h
LDIRT = xfs disk

default install: xfs disk

xfs disk:
	@echo "    [LN]     $@"
	$(Q)$(LN_S) . $@

include $(BUILDRULES)

install-dev: default
	$(INSTALL) -m 755 -d $(PKG_INC_DIR)
	$(INSTALL) -m 644 $(HFILES) $(PKG_INC_DIR)
	$(INSTALL) -m 644 platform_defs.h $(PKG_INC_DIR)

install-qa: install-dev
	$(INSTALL) -m 644 $(QAHFILES) $(PKG_INC_DIR)
