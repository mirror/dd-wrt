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
LSRCFILES = configure.in release.sh README VERSION $(CONFIGURE)

LDIRT = config.log .ltdep .dep config.status config.cache confdefs.h \
	conftest* built .census install.* install-dev.* *.gz \
	autom4te.cache/* libtool include/builddefs include/platform_defs.h

ifeq ($(HAVE_BUILDDEFS), yes)
LDIRDIRT = $(SRCDIR)
LDIRT += $(SRCTAR)
endif

LIB_SUBDIRS = libxfs libxlog libxcmd libhandle libdisk
TOOL_SUBDIRS = copy db estimate fsck fsr growfs io logprint mkfs quota \
		mdrestore repair rtcp m4 man doc po debian

SUBDIRS = include $(LIB_SUBDIRS) $(TOOL_SUBDIRS)

default: include/builddefs include/platform_defs.h
ifeq ($(HAVE_BUILDDEFS), no)
	$(Q)$(MAKE) $(MAKEOPTS) -C . $@
else
	$(Q)$(MAKE) $(MAKEOPTS) $(SUBDIRS)
endif

# tool/lib dependencies
$(LIB_SUBDIRS) $(TOOL_SUBDIRS): include
copy mdrestore: libxfs
db logprint: libxfs libxlog
fsr: libhandle
growfs: libxfs libxcmd
io: libxcmd libhandle
mkfs: libxfs
quota: libxcmd
repair: libxfs libxlog

ifneq ($(ENABLE_BLKID), yes)
mkfs: libdisk
endif

ifeq ($(HAVE_BUILDDEFS), yes)
include $(BUILDRULES)
else
clean:	# if configure hasn't run, nothing to clean
endif

# Recent versions of libtool require the -i option for copying auxiliary
# files (config.sub, config.guess, install-sh, ltmain.sh), while older
# versions will copy those files anyway, and don't understand -i.
LIBTOOLIZE_INSTALL = `libtoolize -n -i >/dev/null 2>/dev/null && echo -i`

configure:
	libtoolize -c $(LIBTOOLIZE_INSTALL) -f
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

install-qa: install $(addsuffix -install-qa,$(SUBDIRS))

%-install:
	@echo "Installing $@"
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install

%-install-dev:
	@echo "Installing $@"
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install-dev

%-install-qa:
	@echo "Installing $@"
	$(Q)$(MAKE) $(MAKEOPTS) -C $* install-qa

distclean: clean
	$(Q)rm -f $(LDIRT)

realclean: distclean
	$(Q)rm -f $(CONFIGURE)

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
	$(Q)$(MAKE) $(MAKEOPTS) $(SRCDIR)
	$(Q)$(MAKE) $(MAKEOPTS) -C po
	$(Q)$(MAKE) $(MAKEOPTS) source-link
	$(Q)cd $(SRCDIR) && dpkg-buildpackage
endif

$(SRCDIR) : $(_FORCE)
	rm -fr $@
	mkdir -p $@

$(SRCTAR) : default $(SRCDIR)
	$(Q)$(MAKE) $(MAKEOPTS) source-link
	unset TAPE; $(TAR) -cf - $(SRCDIR) | $(ZIP) --best > $@ && \
	echo Wrote: $@
