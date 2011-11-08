#
# Copyright (c) 2000-2001,2004 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_logprint

HFILES = logprint.h
CFILES = logprint.c \
	 log_copy.c log_dump.c log_misc.c \
	 log_print_all.c log_print_trans.c

LLDLIBS	= $(LIBXFS) $(LIBXLOG) $(LIBUUID) $(LIBRT) $(LIBPTHREAD)
LTDEPENDENCIES = $(LIBXFS) $(LIBXLOG)
LLDFLAGS = -static

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
install-dev:

-include .dep
