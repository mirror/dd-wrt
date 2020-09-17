# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2001-2003 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

POTHEAD = $(PKG_NAME).pot
# If the user has requested a specific set of translations, only build those.
SUPPORTED_LINGUAS = $(patsubst %.po,%,$(wildcard *.po))
LINGUAS ?= $(SUPPORTED_LINGUAS)
LINGUAS := $(filter $(SUPPORTED_LINGUAS),$(LINGUAS))
LSRCFILES = $(LINGUAS:%=%.po)
LDIRT = $(POTHEAD)

XGETTEXTFILES =	$(LOCALIZED_FILES)

default: $(POTHEAD) $(LINGUAS:%=%.mo)

include $(BUILDRULES)

install: default
	$(INSTALL_LINGUAS)

install-dev install-lib:
