#
# Copyright (c) 2000-2003 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_copy
CFILES = xfs_copy.c
HFILES = xfs_copy.h

LLDLIBS = $(LIBXFS) $(LIBUUID) $(LIBPTHREAD) $(LIBRT)
LTDEPENDENCIES = $(LIBXFS)
LLDFLAGS = -static

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
install-dev:

-include .dep
