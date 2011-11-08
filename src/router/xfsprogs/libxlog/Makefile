#
# Copyright (c) 2000-2002 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxlog.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0

CFILES = xfs_log_recover.c util.c

# don't want to link xfs_repair with a debug libxlog.
DEBUG = -DNDEBUG

default: ltdepend $(LTLIBRARY)

include $(BUILDRULES)

install install-dev install-qa: default
	$(INSTALL_LTLIB)

-include .ltdep
