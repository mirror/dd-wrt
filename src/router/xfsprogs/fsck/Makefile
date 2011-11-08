#
# Copyright (c) 2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LSRCFILES = xfs_fsck.sh

default: $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_ROOT_SBIN_DIR)
	$(INSTALL) -m 755 xfs_fsck.sh $(PKG_ROOT_SBIN_DIR)/fsck.xfs
install-dev:
