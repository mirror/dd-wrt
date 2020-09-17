# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_quota
HFILES = init.h quota.h
CFILES = init.c util.c \
	edit.c free.c linux.c path.c project.c quot.c quota.c report.c state.c

LLDLIBS = $(LIBXCMD) $(LIBFROG)
LTDEPENDENCIES = $(LIBXCMD) $(LIBFROG)
LLDFLAGS = -static

ifeq ($(ENABLE_EDITLINE),yes)
LLDLIBS += $(LIBEDITLINE) $(LIBTERMCAP)
CFLAGS += -DENABLE_EDITLINE
endif

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
install-dev:

-include .dep
