#
# Copyright (c) 2000-2004 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = mkfs.xfs

HFILES =
CFILES = maxtrres.c proto.c xfs_mkfs.c

LLDLIBS += $(LIBXFS) $(LIBXCMD) $(LIBRT) $(LIBPTHREAD) $(LIBBLKID) $(LIBUUID)
LTDEPENDENCIES += $(LIBXFS) $(LIBXCMD)
LLDFLAGS = -static-libtool-libs

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_ROOT_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_ROOT_SBIN_DIR)
install-dev:

-include .dep
