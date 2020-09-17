# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxcmd.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0
# we need a static build even if --disable-static is specified
LTLDFLAGS += -static

CFILES = command.c input.c help.c quit.c

ifeq ($(ENABLE_EDITLINE),yes)
LCFLAGS += -DENABLE_EDITLINE
LTLIBS += $(LIBEDITLINE) $(LIBTERMCAP)
endif

default: ltdepend $(LTLIBRARY)

include $(BUILDRULES)

install install-dev: default

-include .ltdep
