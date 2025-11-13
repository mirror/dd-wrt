# Copyright (C) 1990-2022 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

PACKAGE = sed

SUBDIRS = po . gnulib-tests


# This applies to all parts: gnulib, sed, testsuites
AM_CPPFLAGS =  -I$(top_srcdir) \
               -I$(top_srcdir)/lib \
               -I$(top_builddir)/lib \
               -I$(top_builddir)/sed

AM_CFLAGS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
MAINTAINERCLEANFILES =
MOSTLYCLEANDIRS =
MOSTLYCLEANFILES =
SUFFIXES =
bin_PROGRAMS =
noinst_LIBRARIES =
noinst_DATA =
noinst_HEADERS = basicdefs.h
DISTCLEANFILES =

EXTRA_DIST += BUGS THANKS.in \
  .version				\
  .mailmap ChangeLog-2014 po/ChangeLog-2014 \
  bootstrap bootstrap.conf build-aux/help2man \
  thanks-gen

BUILT_SOURCES += .version
.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

# Arrange so that .tarball-version appears only in the distribution
# tarball, and never in a checked-out repository.
dist-hook: gen-ChangeLog
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version

gen_start_date = 2014-09-06
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -d .git; then				\
	  log_fix="$(srcdir)/build-aux/git-log-fix";			\
	  test -e "$$log_fix"						\
	    && amend_git_log="--amend=$$log_fix"			\
	    || amend_git_log=;						\
	  $(top_srcdir)/build-aux/gitlog-to-changelog			\
	    $$amend_git_log --since=$(gen_start_date) > $(distdir)/cl-t && \
	    { rm -f $(distdir)/ChangeLog &&				\
	      mv $(distdir)/cl-t $(distdir)/ChangeLog; }		\
	fi

# Sort in traditional ASCII order, regardless of the current locale;
# otherwise we may get into trouble with distinct strings that the
# current locale considers to be equal.
ASSORT = LC_ALL=C sort

# Extract all lines up to the first one starting with "##".
prologue = perl -ne '/^\#\#/ and exit; print' $(srcdir)/THANKS.in

THANKS: THANKS.in Makefile.am .mailmap thanks-gen .version
	$(AM_V_GEN)rm -f $@-t $@;					\
	{								\
	  $(prologue); echo;						\
	  { perl -ne '/^$$/.../^$$/ and print' $(srcdir)/THANKS.in	\
	      | grep -v '^$$' | perl -pe 's/  +/\0/';			\
	    git log --pretty=format:'%aN%x00%aE'			\
	      | $(ASSORT) -u;						\
	  } | $(srcdir)/thanks-gen					\
	    | LC_ALL=en_US.UTF-8 sort -f;				\
	  echo;								\
	  printf ';; %s\n' 'Local Variables:' 'coding: utf-8' End:;	\
	} > $@-t && chmod a-w $@-t && mv $@-t $@


AM_DISTCHECK_CONFIGURE_FLAGS = XGETTEXT='$(SHELL) -c : dummy' $(EXTRA_DC_FLAGS)
EXTRA_DC_FLAGS =

full-distcheck:
	$(MAKE) distcheck EXTRA_DC_FLAGS='--enable-regex-tests'
	$(MAKE) distcheck EXTRA_DC_FLAGS='--enable-html'
	$(MAKE) distcheck EXTRA_DC_FLAGS='--disable-i18n'
	$(MAKE) distcheck EXTRA_DC_FLAGS='--disable-nls'
	$(MAKE) distcheck EXTRA_DC_FLAGS='--without-included-gettext'
	@case "$(host)" in \
	  *-linux*|*-gnu*) \
	    echo $(MAKE) distcheck \
              EXTRA_DC_FLAGS=\'--without-included-regex\'; \
	    $(MAKE) distcheck EXTRA_DC_FLAGS='--without-included-regex' \
	    ;; \
	  *) \
	    echo Skipping check --without-included-regex \
	    ;; \
	esac

include $(top_srcdir)/lib/local.mk
include $(top_srcdir)/sed/local.mk
include $(top_srcdir)/doc/local.mk
include $(top_srcdir)/testsuite/local.mk

# Remove the generated sed.1 file, but only for a non-srcdir build.
# Without this, 'make distcheck's final comparison would fail.
# Also remove lib/.deps/{getfilecon,obstack}.Po,
# which are left behind by bleeding edge automake.
# Without this, 'make distcheck's final comparison would fail.
distclean-local:
	test x$(srcdir) = x$(builddir) || rm -f $(dist_man_MANS)
	rm -f lib/.deps/getfilecon.Po lib/.deps/obstack.Po

# Shortcut targets to make it easier to run (very) expensive tests.
check-expensive:
	$(MAKE) check RUN_EXPENSIVE_TESTS=yes
check-very-expensive:
	$(MAKE) check-expensive RUN_VERY_EXPENSIVE_TESTS=yes
