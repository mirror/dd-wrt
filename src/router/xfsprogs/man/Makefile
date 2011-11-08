#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

SUBDIRS = man3 man5 man8

default : $(SUBDIRS)

install : $(addsuffix -install,$(SUBDIRS))

install-dev : $(addsuffix -install-dev,$(SUBDIRS))

%-install:
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install

%-install-dev:
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install-dev

include $(BUILDRULES)
