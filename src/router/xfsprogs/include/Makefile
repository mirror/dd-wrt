# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2006 Silicon Graphics, Inc.
# All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LIBHFILES = libxfs.h \
	libxlog.h \
	libxcmd.h \
	atomic.h \
	bitops.h \
	cache.h \
	hlist.h \
	kmem.h \
	list.h \
	parent.h \
	spinlock.h \
	xfs_inode.h \
	xfs_log_recover.h \
	xfs_metadump.h \
	xfs_mount.h \
	xfs_quota_defs.h \
	xfs_sb.h \
	xfs_shared.h \
	xfs_trace.h \
	xfs_trans.h \
	command.h \
	input.h \
	platform_defs.h

HFILES = handle.h \
	jdm.h \
	linux.h \
	xfs.h \
	xqm.h \
	xfs_fs_compat.h \
	xfs_arch.h

LSRCFILES = platform_defs.h.in builddefs.in buildmacros buildrules install-sh
LSRCFILES += $(DKHFILES) $(LIBHFILES)
LDIRT = disk
LDIRDIRT = xfs

default: disk

disk:
	@echo "    [LN]     $@"
	$(Q)$(LN_S) . $@

include $(BUILDRULES)

# set up include/xfs header directory
install-headers: $(addsuffix -hdrs, $(DKHFILES) $(HFILES))

%-hdrs:
	$(Q)$(LN_S) -f $(CURDIR)/$* xfs/$*

install: default
	$(INSTALL) -m 755 -d $(PKG_INC_DIR)

install-dev: install
	$(INSTALL) -m 644 $(HFILES) $(PKG_INC_DIR)
