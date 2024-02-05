# SPDX-License-Identifier: GPL-2.0
#
# Copyright (c) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
  BUILD_VERBOSE = 0
endif

ifeq ($(BUILD_VERBOSE),1)
  Q =
else
  Q = @
endif

CHECK=sparse
CHECK_OPTS=-Wsparse-all -Wbitwise -Wno-transparent-union -Wno-return-void -Wno-undef \
	-Wno-non-pointer-null -D__CHECK_ENDIAN__ -D__linux__

ifeq ("$(origin C)", "command line")
  CHECK_CMD=$(CHECK) $(CHECK_OPTS)
  CHECKSRC=$(C)
else
  CHECK_CMD=@true
  CHECKSRC=0
endif

export CHECK_CMD CHECKSRC

MAKEOPTS = --no-print-directory Q=$(Q)

TOPDIR = .
HAVE_BUILDDEFS = $(shell test -f $(TOPDIR)/include/builddefs && echo yes || echo no)

ifeq ($(HAVE_BUILDDEFS), yes)
include $(TOPDIR)/include/builddefs
endif

SRCDIR = $(PKG_NAME)-$(PKG_VERSION)
SRCTAR = $(PKG_NAME)-$(PKG_VERSION).tar.gz
SRCTARXZ = $(PKG_NAME)-$(PKG_VERSION).tar.xz

CONFIGURE = aclocal.m4 configure config.guess config.sub install-sh ltmain.sh
LSRCFILES = configure.ac release.sh README VERSION $(CONFIGURE)
SRCTARINC = m4/libtool.m4 m4/lt~obsolete.m4 m4/ltoptions.m4 m4/ltsugar.m4 \
           m4/ltversion.m4 po/xfsprogs.pot .gitcensus $(CONFIGURE)
LDIRT = config.log .ltdep .dep config.status config.cache confdefs.h \
	conftest* built .census install.* install-dev.* *.gz *.xz \
	autom4te.cache/* libtool include/builddefs include/platform_defs.h

ifeq ($(HAVE_BUILDDEFS), yes)
LDIRDIRT = $(SRCDIR)
LDIRT += $(SRCTAR) $(SRCTARXZ)
endif

# header install rules to populate include/xfs correctly
HDR_SUBDIRS = include libxfs

LIBFROG_SUBDIR = libfrog
DLIB_SUBDIRS = libxlog libxcmd libhandle
LIB_SUBDIRS = libxfs $(DLIB_SUBDIRS)
TOOL_SUBDIRS = copy db estimate fsck fsr growfs io logprint mkfs quota \
		mdrestore repair rtcp m4 man doc debian spaceman

ifeq ("$(ENABLE_SCRUB)","yes")
TOOL_SUBDIRS += scrub
endif

ifneq ("$(XGETTEXT)","")
TOOL_SUBDIRS += po
endif

# include is listed last so it is processed last in clean rules.
SUBDIRS = $(LIBFROG_SUBDIR) $(LIB_SUBDIRS) $(TOOL_SUBDIRS) include

default: include/builddefs include/platform_defs.h
ifeq ($(HAVE_BUILDDEFS), no)
	$(Q)$(MAKE) $(MAKEOPTS) -C . $@
else
	$(Q)$(MAKE) $(MAKEOPTS) headers
	$(Q)$(MAKE) $(MAKEOPTS) $(SUBDIRS)
endif

# tool/lib dependencies
# note: include/xfs is set up by libxfs, too, so everything is dependent on it.
$(LIBFROG_SUBDIR): include
$(LIB_SUBDIRS) $(TOOL_SUBDIRS): include libfrog
$(DLIB_SUBDIRS) $(TOOL_SUBDIRS): libxfs
db logprint: libxlog
fsr: libhandle
growfs: libxcmd
io: libxcmd libhandle
quota: libxcmd
repair: libxlog libxcmd
copy: libxlog
mkfs: libxcmd
spaceman: libxcmd
scrub: libhandle libxcmd
rtcp: libfrog

ifeq ($(HAVE_BUILDDEFS), yes)
include $(BUILDRULES)
else
clean:	# if configure hasn't run, nothing to clean
endif

configure: configure.ac
	libtoolize -c -i -f
	cp include/install-sh .
	aclocal -I m4
	autoconf

include/builddefs: configure
	./configure $$LOCAL_CONFIGURE_OPTIONS

include/platform_defs.h: include/builddefs
## Recover from the removal of $@
	@if test -f $@; then :; else \
		rm -f include/builddefs; \
		$(MAKE) $(MAKEOPTS) $(AM_MAKEFLAGS) include/builddefs; \
	fi

install: $(addsuffix -install,$(SUBDIRS))
	$(INSTALL) -m 755 -d $(PKG_DOC_DIR)
	$(INSTALL) -m 644 README $(PKG_DOC_DIR)

install-dev: $(addsuffix -install-dev,$(SUBDIRS))

%-install:
	@echo "Installing $@"
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install

%-install-dev:
	@echo "Installing $@"
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install-dev

distclean: clean
	$(Q)rm -f $(LDIRT)

realclean: distclean
	$(Q)rm -f $(CONFIGURE) .gitcensus

#
# All this gunk is to allow for a make dist on an unconfigured tree
#
dist: include/builddefs include/platform_defs.h default
ifeq ($(HAVE_BUILDDEFS), no)
	$(Q)$(MAKE) $(MAKEOPTS) -C . $@
else
	$(Q)$(MAKE) $(MAKEOPTS) $(SRCTAR)
endif

deb: include/builddefs include/platform_defs.h
ifeq ($(HAVE_BUILDDEFS), no)
	$(Q)$(MAKE) $(MAKEOPTS) -C . $@
else
	# need to build translations before the source tarball
	$(Q)$(MAKE) $(MAKEOPTS) -C po
	$(Q)$(MAKE) $(MAKEOPTS) $(SRCDIR)
	$(Q)rm -f $(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
	$(Q)$(LN_S) $(SRCTAR) $(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
	$(Q)cd $(SRCDIR) && dpkg-buildpackage $$LOCAL_DPKG_OPTIONS # -sa -S
endif

$(SRCDIR) : $(_FORCE) $(SRCTAR)
	rm -fr $@
	$(Q)$(TAR) -zxvf $(SRCTAR)

$(SRCTAR) : default $(SRCTARINC) .gitcensus
	$(Q)$(TAR) --transform "s,^,$(SRCDIR)/," -zcf $(SRCDIR).tar.gz  \
	   `cat .gitcensus` $(SRCTARINC)
	echo Wrote: $@

$(SRCTARXZ) : default $(SRCTARINC) .gitcensus
	$(Q)$(TAR) --transform "s,^,$(SRCDIR)/," -Jcf $(SRCDIR).tar.xz  \
	   `cat .gitcensus` $(SRCTARINC)
	echo Wrote: $@

.gitcensus: $(_FORCE)
	$(Q)if test -d .git; then \
	  git ls-files > .gitcensus && echo "new .gitcensus"; \
	fi
