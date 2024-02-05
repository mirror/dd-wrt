# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2003-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

CONFIGURE = \
	libtool.m4 \
	ltoptions.m4 \
	ltsugar.m4 \
	ltversion.m4 \
	lt~obsolete.m4

LSRCFILES = \
	manual_format.m4 \
	package_blkid.m4 \
	package_devmapper.m4 \
	package_globals.m4 \
	package_attr.m4 \
	package_libcdev.m4 \
	package_pthread.m4 \
	package_sanitizer.m4 \
	package_services.m4 \
	package_types.m4 \
	package_icu.m4 \
	package_urcu.m4 \
	package_utilies.m4 \
	package_uuiddev.m4 \
	multilib.m4 \
	$(CONFIGURE)

default:

include $(BUILDRULES)

install install-dev install-lib: default

realclean: distclean
	rm -f $(CONFIGURE)
