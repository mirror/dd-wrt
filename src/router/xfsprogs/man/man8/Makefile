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
MAN_DEST	= $(PKG_MAN_DIR)/man$(MAN_SECTION)
LSRCFILES	= $(MAN_PAGES)

default : $(MAN_PAGES)

include $(BUILDRULES)

install : default
	$(INSTALL) -m 755 -d $(MAN_DEST)
	$(INSTALL_MAN)
install-dev :
