# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_io
LSRCFILES = xfs_bmap.sh xfs_freeze.sh xfs_mkfile.sh
HFILES = init.h io.h
CFILES = init.c \
	attr.c bmap.c bulkstat.c crc32cselftest.c cowextsize.c encrypt.c \
	file.c freeze.c fsuuid.c fsync.c getrusage.c imap.c inject.c label.c \
	link.c mmap.c open.c parent.c pread.c prealloc.c pwrite.c reflink.c \
	resblks.c scrub.c seek.c shutdown.c stat.c swapext.c sync.c \
	truncate.c utimes.c

LLDLIBS = $(LIBXCMD) $(LIBHANDLE) $(LIBFROG) $(LIBPTHREAD) $(LIBUUID)
LTDEPENDENCIES = $(LIBXCMD) $(LIBHANDLE) $(LIBFROG)
LLDFLAGS = -static-libtool-libs

ifeq ($(HAVE_FADVISE),yes)
CFILES += fadvise.c
LCFLAGS += -DHAVE_FADVISE
else
LSRCFILES += fadvise.c
endif

ifeq ($(HAVE_MADVISE),yes)
CFILES += madvise.c
LCFLAGS += -DHAVE_MADVISE
else
LSRCFILES += madvise.c
endif

ifeq ($(HAVE_MINCORE),yes)
CFILES += mincore.c
LCFLAGS += -DHAVE_MINCORE
else
LSRCFILES += mincore.c
endif

ifeq ($(HAVE_SENDFILE),yes)
CFILES += sendfile.c
LCFLAGS += -DHAVE_SENDFILE
else
LSRCFILES += sendfile.c
endif

ifeq ($(HAVE_FIEMAP),yes)
CFILES += fiemap.c
LCFLAGS += -DHAVE_FIEMAP
else
LSRCFILES += fiemap.c
endif

ifeq ($(HAVE_COPY_FILE_RANGE),yes)
CFILES += copy_file_range.c
LCFLAGS += -DHAVE_COPY_FILE_RANGE
endif

ifeq ($(HAVE_SYNC_FILE_RANGE),yes)
CFILES += sync_file_range.c
LCFLAGS += -DHAVE_SYNC_FILE_RANGE
endif

ifeq ($(HAVE_SYNCFS),yes)
LCFLAGS += -DHAVE_SYNCFS
endif

ifeq ($(ENABLE_EDITLINE),yes)
LLDLIBS += $(LIBEDITLINE) $(LIBTERMCAP)
endif

ifeq ($(HAVE_FALLOCATE),yes)
LCFLAGS += -DHAVE_FALLOCATE
endif

# Also implies PWRITEV
ifeq ($(HAVE_PREADV),yes)
LCFLAGS += -DHAVE_PREADV -DHAVE_PWRITEV
endif

ifeq ($(HAVE_PWRITEV2),yes)
LCFLAGS += -DHAVE_PWRITEV2
endif

ifeq ($(HAVE_READDIR),yes)
CFILES += readdir.c
LCFLAGS += -DHAVE_READDIR
endif

ifeq ($(HAVE_MREMAP),yes)
LCFLAGS += -DHAVE_MREMAP
endif

ifeq ($(HAVE_MAP_SYNC),yes)
LCFLAGS += -DHAVE_MAP_SYNC
endif

ifeq ($(HAVE_DEVMAPPER),yes)
CFILES += log_writes.c
LLDLIBS += $(LIBDEVMAPPER)
LCFLAGS += -DHAVE_DEVMAPPER
endif

ifeq ($(HAVE_GETFSMAP),yes)
CFILES += fsmap.c
endif

ifeq ($(HAVE_STATFS_FLAGS),yes)
LCFLAGS += -DHAVE_STATFS_FLAGS
endif

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 xfs_bmap.sh $(PKG_SBIN_DIR)/xfs_bmap
	$(LTINSTALL) -m 755 xfs_freeze.sh $(PKG_SBIN_DIR)/xfs_freeze
	$(LTINSTALL) -m 755 xfs_mkfile.sh $(PKG_SBIN_DIR)/xfs_mkfile
install-dev:

-include .dep
