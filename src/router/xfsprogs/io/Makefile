#
# Copyright (c) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_io
LSRCFILES = xfs_bmap.sh xfs_freeze.sh xfs_mkfile.sh
HFILES = init.h io.h
CFILES = init.c \
	attr.c bmap.c cowextsize.c encrypt.c file.c freeze.c fsync.c \
	getrusage.c imap.c link.c mmap.c open.c parent.c pread.c prealloc.c \
	pwrite.c reflink.c seek.c shutdown.c stat.c sync.c truncate.c utimes.c

LLDLIBS = $(LIBXCMD) $(LIBHANDLE) $(LIBPTHREAD)
LTDEPENDENCIES = $(LIBXCMD) $(LIBHANDLE)
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

ifeq ($(PKG_PLATFORM),irix)
LSRCFILES += inject.c resblks.c
else
CFILES += inject.c resblks.c
LCFLAGS += -DHAVE_INJECT -DHAVE_RESBLKS
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

ifeq ($(ENABLE_READLINE),yes)
LLDLIBS += $(LIBREADLINE) $(LIBTERMCAP)
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

# On linux we get fsmap from the system or define it ourselves
# so include this based on platform type.  If this reverts to only
# the autoconf check w/o local definition, change to testing HAVE_GETFSMAP
ifeq ($(PKG_PLATFORM),linux)
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
