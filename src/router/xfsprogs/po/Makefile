#
# Copyright (c) 2001-2003 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

POTHEAD = $(PKG_NAME).pot
LINGUAS = de pl
LSRCFILES = $(LINGUAS:%=%.po)
LDIRT = $(POTHEAD)

XGETTEXTFILES =	$(LOCALIZED_FILES)

default: $(POTHEAD) $(LINGUAS:%=%.mo)

include $(BUILDRULES)

install: default
	$(INSTALL_LINGUAS)

install-dev install-lib:
