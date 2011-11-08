#
# Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_growfs

CFILES = xfs_growfs.c

LLDLIBS = $(LIBXFS) $(LIBXCMD) $(LIBUUID) $(LIBRT) $(LIBPTHREAD)
ifeq ($(ENABLE_READLINE),yes)
LLDLIBS += $(LIBREADLINE) $(LIBTERMCAP)
endif

ifeq ($(ENABLE_EDITLINE),yes)
LLDLIBS += $(LIBEDITLINE) $(LIBTERMCAP)
endif

LTDEPENDENCIES = $(LIBXFS) $(LIBXCMD)
LLDFLAGS = -static
LSRCFILES = xfs_info.sh

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
	$(INSTALL) -m 755 xfs_info.sh $(PKG_SBIN_DIR)/xfs_info
install-dev:

-include .dep
