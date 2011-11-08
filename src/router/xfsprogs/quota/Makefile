#
# Copyright (c) 2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_quota
HFILES = init.h quota.h
CFILES = init.c util.c \
	edit.c free.c path.c project.c quot.c quota.c report.c state.c

CFILES += $(PKG_PLATFORM).c
PCFILES = darwin.c freebsd.c irix.c linux.c
LSRCFILES = $(shell echo $(PCFILES) | sed -e "s/$(PKG_PLATFORM).c//g")

LLDLIBS = $(LIBXCMD)
LTDEPENDENCIES = $(LIBXCMD)
LLDFLAGS = -static

ifeq ($(ENABLE_READLINE),yes)
LLDLIBS += $(LIBREADLINE) $(LIBTERMCAP)
CFLAGS += -DENABLE_READLINE
endif

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
