#
# Copyright (c) 2000-2004 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = mkfs.xfs
FSTYP = fstyp

HFILES = xfs_mkfs.h
CFILES = maxtrres.c proto.c xfs_mkfs.c

ifeq ($(ENABLE_BLKID),yes)
LLDLIBS += $(LIBBLKID)
CFLAGS += -DENABLE_BLKID
else
LLDLIBS += $(LIBDISK)
LTDEPENDENCIES += $(LIBDISK)
endif

LLDLIBS += $(LIBXFS) $(LIBUUID) $(LIBRT) $(LIBPTHREAD)
LTDEPENDENCIES += $(LIBXFS)
LLDFLAGS = -static

LSRCFILES = $(FSTYP).c
LDIRT = $(FSTYP)

default: depend $(LTCOMMAND)

ifneq ($(ENABLE_BLKID),yes)
default: $(FSTYP)
endif

include $(BUILDRULES)

$(FSTYP):
	@echo "    [CC]     $@"
	$(Q)$(LTLINK) $@.c -o $@ $(CFLAGS) $(LDFLAGS) $(LIBDISK) $(PLDLIBS)

install: default
	$(INSTALL) -m 755 -d $(PKG_ROOT_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_ROOT_SBIN_DIR)
install-dev:

-include .dep
