# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTCOMMAND = xfs_db

HFILES = addr.h agf.h agfl.h agi.h attr.h attrshort.h bit.h block.h bmap.h \
	btblock.h bmroot.h check.h command.h crc.h debug.h \
	dir2.h dir2sf.h dquot.h echo.h faddr.h field.h \
	flist.h fprint.h frag.h freesp.h hash.h help.h init.h inode.h input.h \
	io.h logformat.h malloc.h metadump.h output.h print.h quit.h sb.h \
	sig.h strvec.h text.h type.h write.h attrset.h symlink.h fsmap.h \
	fuzz.h obfuscate.h
CFILES = $(HFILES:.h=.c) btdump.c btheight.c convert.c info.c iunlink.c namei.c \
	timelimit.c
LSRCFILES = xfs_admin.sh xfs_ncheck.sh xfs_metadump.sh

LLDLIBS	= $(LIBXFS) $(LIBXLOG) $(LIBFROG) $(LIBUUID) $(LIBRT) $(LIBURCU) \
	  $(LIBPTHREAD)
LTDEPENDENCIES = $(LIBXFS) $(LIBXLOG) $(LIBFROG)
LLDFLAGS += -static-libtool-libs

ifeq ($(ENABLE_EDITLINE),yes)
LLDLIBS += $(LIBEDITLINE) $(LIBTERMCAP)
CFLAGS += -DENABLE_EDITLINE
endif

default: depend $(LTCOMMAND)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_SBIN_DIR)
	$(LTINSTALL) -m 755 $(LTCOMMAND) $(PKG_SBIN_DIR)
	$(INSTALL) -m 755 xfs_admin.sh $(PKG_SBIN_DIR)/xfs_admin
	$(INSTALL) -m 755 xfs_ncheck.sh $(PKG_SBIN_DIR)/xfs_ncheck
	$(INSTALL) -m 755 xfs_metadump.sh $(PKG_SBIN_DIR)/xfs_metadump
install-dev:

-include .dep
