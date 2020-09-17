# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017 Oracle.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libfrog.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0
# we need a static build even if --disable-static is specified
LTLDFLAGS += -static

CFILES = \
avl64.c \
bitmap.c \
bulkstat.c \
convert.c \
crc32.c \
fsgeom.c \
list_sort.c \
linux.c \
logging.c \
paths.c \
projects.c \
ptvar.c \
radix-tree.c \
scrub.c \
topology.c \
util.c \
workqueue.c

HFILES = \
avl64.h \
bulkstat.h \
bitmap.h \
convert.h \
crc32c.h \
crc32cselftest.h \
crc32defs.h \
crc32table.h \
fsgeom.h \
logging.h \
paths.h \
projects.h \
ptvar.h \
radix-tree.h \
scrub.h \
topology.h \
workqueue.h

LSRCFILES += gen_crc32table.c

ifeq ($(HAVE_GETMNTENT),yes)
LCFLAGS += -DHAVE_GETMNTENT
endif

LDIRT = gen_crc32table crc32table.h crc32selftest

default: crc32selftest ltdepend $(LTLIBRARY)

crc32table.h: gen_crc32table.c crc32defs.h
	@echo "    [CC]     gen_crc32table"
	$(Q) $(BUILD_CC) $(BUILD_CFLAGS) -o gen_crc32table $<
	@echo "    [GENERATE] $@"
	$(Q) ./gen_crc32table > crc32table.h

# The selftest binary will return an error if it fails. This is made a
# dependency of the build process so that we refuse to build the tools on broken
# systems/architectures. Hence we make sure that xfsprogs will never use a
# busted CRC calculation at build time and hence avoid putting bad CRCs down on
# disk.
crc32selftest: gen_crc32table.c crc32table.h crc32.c crc32defs.h
	@echo "    [TEST]    CRC32"
	$(Q) $(BUILD_CC) $(BUILD_CFLAGS) -D CRC32_SELFTEST=1 crc32.c -o $@
	$(Q) ./$@

include $(BUILDRULES)

install install-dev: default

-include .ltdep
