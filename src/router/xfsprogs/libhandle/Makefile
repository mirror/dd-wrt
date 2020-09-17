# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2004 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libhandle.la
LT_CURRENT = 1
LT_REVISION = 3
LT_AGE = 0

LTLDFLAGS += -Wl,--version-script,libhandle.sym

CFILES = handle.c jdm.c
LSRCFILES = libhandle.sym

default: ltdepend $(LTLIBRARY)

include $(BUILDRULES)

install: default
	$(INSTALL_LTLIB)

install-dev: default
	$(INSTALL_LTLIB_DEV)

-include .ltdep
