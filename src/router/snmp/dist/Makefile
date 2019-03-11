# ======================================================================
# $Source$
# $Revision$
# $Date$
# $Author$
# $Name$
# $State$
# ======================================================================

# Makefile for generating rpm package files (.rpm) for net-snmp.

# GNU 'make'-specific features are used in this makefile.
# Other makes may or may not work with it.
# See http://www.gnu.org/manual/make/index.html for details.

SHELL = /bin/sh

# Define the RPM package name
NAME = net-snmp

# Define the default RPM release number. Set by Version-Munge.pl
VERSION = 5.8

VERSION_TAG=`echo Ext-$(VERSION) | sed 's/\./-/g;'`

WITH_PERL=1
EMBED_PERL=1

# Define the RPM architecture, i.e., 'ppc', 'i386', etc.
ARCH = `uname -i`

# Directory structure expected by 'rpm' program
RPM_BASE := $(PWD)/rpm

RPM_TREE := \
    $(RPM_BASE)/BUILD \
    $(RPM_BASE)/RPMS \
    $(RPM_BASE)/SOURCES \
    $(RPM_BASE)/SPECS \
    $(RPM_BASE)/SRPMS

.PHONY:	all clean rpm_files binary_rpm source_rpm all_rpm

all:	all_rpm_files

# Delete all generated files.
clean:
	rm -rf $(RPM_BASE)

# --------------------------------------------------------------------
# Macros and rules for updating net-snmp-x.x.x.tar.gz.
# 'rpm' expects a compressed archive (.tar.gz) of the source directory
# to exist (in the rpm/SOURCES directory) before it is run.
# --------------------------------------------------------------------
CVSROOT = :pserver:anonymous@net-snmp.cvs.sourceforge.net:/cvsroot/net-snmp

PKG_VER := $(NAME)-$(VERSION)

GZIP_TAR := $(RPM_BASE)/SOURCES/$(PKG_VER).tar.gz

gzip_tar:	$(GZIP_TAR)

$(GZIP_TAR):
	@echo "Creating a compressed archive of the package's source files..."
	(cd $(RPM_BASE)/SOURCES; \
	cvs -d$(CVSROOT) login; \
	cvs -z3 -d$(CVSROOT) export -r$(VERSION_TAG) $(NAME); \
	mv $(NAME) $(PKG_VER); \
	tar cf $(PKG_VER).tar $(PKG_VER); \
	gzip $(PKG_VER).tar)
	@echo "A compressed archive of the package's source-file tree has been created."

# --------------------------------------------------------------------
# Macros and rules for updating the binary and source RPM package files.
# --------------------------------------------------------------------
# Redefine '%_topdir' to allow an RPM package file to be built anywhere,
# instead of in the /usr/src/redhat directory tree.
#
# Don't want 'rpmbuild' to strip your executable programs?
# Then add this line:
#   --define='_os_install_post /usr/lib/rpm/brp-compress' \
# to the RPM_OPTS macro definition.  This will eliminate the 
# stripping of binaries, but still retain the compression of
# manual pages.
#
ifeq ($(WITH_PERL),0)
PERLOPTS = --without perl_modules
endif
ifeq ($(EMBED_PERL),0)
PERLOPTS += --without embedded_perl
endif

RPM_OPTS := \
    --define='_topdir $(RPM_BASE)' \
    --define='_includedir /usr/include/net-snmp' \
    --target $(ARCH) $(PERLOPTS)

SPEC_FILE  := $(NAME).spec
RPM_SPEC   := $(RPM_BASE)/SPECS/$(SPEC_FILE)

BINARY_RPM := $(RPM_BASE)/RPMS/$(ARCH)/$(PKG_VER)-$(RELEASE).$(ARCH).rpm
SOURCE_RPM := $(RPM_BASE)/SRPMS/$(PKG_VER)-$(RELEASE).src.rpm

rpm_files:	$(GZIP_TAR) all_rpm_files
binary_rpm:	$(BINARY_RPM)
source_rpm:	$(SOURCE_RPM)
rpm_tree: $(RPM_TREE)

all_rpm_files:	$(RPM_TREE) $(RPM_SPEC)
	(cd $(RPM_BASE)/SPECS; \
	rpmbuild -ba $(RPM_OPTS) $(SPEC_FILE))

$(BINARY_RPM):	$(RPM_TREE) $(RPM_SPEC)
	(cd $(RPM_BASE)/SPECS; \
	rpmbuild -bb $(RPM_OPTS) $(SPEC_FILE))

$(SOURCE_RPM):	$(RPM_TREE) $(RPM_SPEC)
	(cd $(RPM_BASE)/SPECS; \
	rpmbuild -bs $(RPM_OPTS) $(SPEC_FILE))

$(RPM_SPEC):	$(RPM_BASE)/SPECS $(SPEC_FILE)
	cp $(SPEC_FILE) $@

$(RPM_TREE):
	mkdir -p $@

$(SPEC_FILE):
	@echo "ERROR: missing '$(SPEC_FILE)' in the current directory"
	@exit 1

FORCE:

# === End of Makefile === #
