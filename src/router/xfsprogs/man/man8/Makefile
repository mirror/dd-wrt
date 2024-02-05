#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ../..
include $(TOPDIR)/include/builddefs

MAN_SECTION	= 8

ifneq ("$(ENABLE_SCRUB)","yes")
  MAN_PAGES = $(filter-out xfs_scrub%,$(shell echo *.$(MAN_SECTION)))
else
  MAN_PAGES = $(shell echo *.$(MAN_SECTION))
endif
MAN_PAGES	+= mkfs.xfs.8
MAN_DEST	= $(PKG_MAN_DIR)/man$(MAN_SECTION)
LSRCFILES	= $(MAN_PAGES)
DIRT		= mkfs.xfs.8

default : $(MAN_PAGES)

include $(BUILDRULES)

install : default
	$(INSTALL) -m 755 -d $(MAN_DEST)
	$(INSTALL_MAN)

mkfs.xfs.8: mkfs.xfs.8.in
	@echo "    [SED]    $@"
	$(Q)$(SED) -e 's|@mkfs_cfg_dir@|$(MKFS_CFG_DIR)|g' < $^ > $@

install-dev :
