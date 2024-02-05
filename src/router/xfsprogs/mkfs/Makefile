# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2004 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = mkfs.xfs

HFILES =
CFILES = proto.c xfs_mkfs.c ini.c
CFGFILES = \
	dax_x86_64.conf \
	lts_4.19.conf \
	lts_5.4.conf \
	lts_5.10.conf \
	lts_5.15.conf \
	lts_6.1.conf \
	lts_6.6.conf

LLDLIBS += $(LIBXFS) $(LIBXCMD) $(LIBFROG) $(LIBRT) $(LIBBLKID) \
	$(LIBUUID) $(LIBINIH) $(LIBURCU) $(LIBPTHREAD)
LTDEPENDENCIES += $(LIBXFS) $(LIBXCMD) $(LIBFROG)
LLDFLAGS = -static-libtool-libs

default: depend $(LTCOMMAND) $(CFGFILES)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_ROOT_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_ROOT_SBIN_DIR)
	$(INSTALL) -m 755 -d $(MKFS_CFG_DIR)
	$(INSTALL) -m 644 $(CFGFILES) $(MKFS_CFG_DIR)

install-dev:

-include .dep
