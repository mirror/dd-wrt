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

MAKEOPTS = --no-print-directory Q=$(Q)

TOPDIR = .
HAVE_BUILDDEFS = $(shell test -f $(TOPDIR)/include/builddefs && echo yes || echo no)

ifeq ($(HAVE_BUILDDEFS), yes)
include $(TOPDIR)/include/builddefs
endif

SRCDIR = $(PKG_NAME)-$(PKG_VERSION)
SRCTAR = $(PKG_NAME)-$(PKG_VERSION).tar.gz

CONFIGURE = aclocal.m4 configure config.guess config.sub install-sh ltmain.sh
LSRCFILES = configure.ac release.sh README VERSION $(CONFIGURE)
SRCTARINC = m4/libtool.m4 m4/lt~obsolete.m4 m4/ltoptions.m4 m4/ltsugar.m4 \
           m4/ltversion.m4 po/xfsprogs.pot .gitcensus $(CONFIGURE)
LDIRT = config.log .ltdep .dep config.status config.cache confdefs.h \
	conftest* built .census install.* install-dev.* *.gz \
	autom4te.cache/* libtool include/builddefs include/platform_defs.h

ifeq ($(HAVE_BUILDDEFS), yes)
LDIRDIRT = $(SRCDIR)
LDIRT += $(SRCTAR)
endif

# header install rules to populate include/xfs correctly
HDR_SUBDIRS = include libxfs

DLIB_SUBDIRS = libxlog libxcmd libhandle
LIB_SUBDIRS = libxfs $(DLIB_SUBDIRS)
TOOL_SUBDIRS = copy db estimate fsck growfs io logprint mkfs quota \
		mdrestore repair rtcp m4 man doc debian

ifneq ("$(PKG_PLATFORM)","darwin")
TOOL_SUBDIRS += fsr
endif

ifneq ("$(XGETTEXT)","")
TOOL_SUBDIRS += po
endif

# If we are on OS X, use glibtoolize from MacPorts, as OS X doesn't have
# libtoolize binary itself.
LIBTOOLIZE_TEST=$(shell libtoolize --version >/dev/null 2>&1 && echo found)
LIBTOOLIZE_BIN=libtoolize
ifneq ("$(LIBTOOLIZE_TEST)","found")
LIBTOOLIZE_BIN=glibtoolize
endif

# include is listed last so it is processed last in clean rules.
SUBDIRS = $(LIB_SUBDIRS) $(TOOL_SUBDIRS) include

default: include/builddefs include/platform_defs.h
ifeq ($(HAVE_BUILDDEFS), no)
	$(Q)$(MAKE) $(MAKEOPTS) -C . $@
else
	$(Q)$(MAKE) $(MAKEOPTS) headers
	$(Q)$(MAKE) $(MAKEOPTS) $(SUBDIRS)
endif

# tool/lib dependencies
# note: include/xfs is set up by libxfs, too, so everything is dependent on it.
$(LIB_SUBDIRS) $(TOOL_SUBDIRS): include
$(DLIB_SUBDIRS) $(TOOL_SUBDIRS): libxfs
db logprint: libxlog
fsr: libhandle
growfs: libxcmd
io: libxcmd libhandle
quota: libxcmd
repair: libxlog libxcmd
copy: libxlog
mkfs: libxcmd

ifeq ($(HAVE_BUILDDEFS), yes)
include $(BUILDRULES)
else
clean:	# if configure hasn't run, nothing to clean
endif


# Recent versions of libtool require the -i option for copying auxiliary
# files (config.sub, config.guess, install-sh, ltmain.sh), while older
# versions will copy those files anyway, and don't understand -i.
LIBTOOLIZE_INSTALL = `$(LIBTOOLIZE_BIN) -n -i >/dev/null 2>/dev/null && echo -i`

configure:
	$(LIBTOOLIZE_BIN) -c $(LIBTOOLIZE_INSTALL) -f
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
	$(Q)cd $(SRCDIR) && dpkg-buildpackage
endif

$(SRCDIR) : $(_FORCE) $(SRCTAR)
	rm -fr $@
	$(Q)$(TAR) -zxvf $(SRCTAR)

$(SRCTAR) : default $(SRCTARINC) .gitcensus
	$(Q)$(TAR) --transform "s,^,$(SRCDIR)/," -zcf $(SRCDIR).tar.gz  \
	   `cat .gitcensus` $(SRCTARINC)
	echo Wrote: $@

.gitcensus: $(_FORCE)
	$(Q)if test -d .git; then \
	  git ls-files > .gitcensus && echo "new .gitcensus"; \
	fi
