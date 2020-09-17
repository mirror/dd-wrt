# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_fsr
CFILES = xfs_fsr.c
LLDLIBS = $(LIBHANDLE) $(LIBFROG) $(LIBPTHREAD) $(LIBBLKID)
LTDEPENDENCIES = $(LIBHANDLE) $(LIBFROG)
LLDFLAGS = -static-libtool-libs

ifeq ($(HAVE_GETMNTENT),yes)
LCFLAGS += -DHAVE_GETMNTENT
endif

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
install-dev:

-include .dep
