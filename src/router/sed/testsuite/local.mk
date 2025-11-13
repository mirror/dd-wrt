# Copyright (C) 2016-2022 Free Software Foundation, Inc.

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

CLEANFILES += tmp* core *.core $(EXTRA_PROGRAMS) *.*out *.log

TEST_EXTENSIONS = .sh .pl

if HAVE_PERL
TESTSUITE_PERL = $(PERL)
else
TESTSUITE_PERL = $(SHELL) $(srcdir)/testsuite/no-perl
endif

# Options passed to the perl invocations running the perl test scripts.
TESTSUITE_PERL_OPTIONS = -w -I$(srcdir)/testsuite -MCuSkip -MCoreutils
# '$f' is set by the Automake-generated test harness to the path of the
# current test script stripped of VPATH components, and is used by the
# CuTmpdir module to determine the name of the temporary files to be
# used.  Note that $f is a shell variable, not a make macro, so the use
# of '$$f' below is correct, and not a typo.
TESTSUITE_PERL_OPTIONS += -M"CuTmpdir qw($$f)"

SH_LOG_COMPILER = $(SHELL)
PL_LOG_COMPILER = $(TESTSUITE_PERL) $(TESTSUITE_PERL_OPTIONS)

# Ensure that anything not covered by the above evokes failure.
LOG_COMPILER = false

# Put new, init.sh-using tests here, so that each name
# is listed in only one place.

T =					\
  testsuite/misc.pl			\
  testsuite/bug32082.sh		\
  testsuite/bug32271-1.sh		\
  testsuite/bug32271-2.sh		\
  testsuite/cmd-l.sh			\
  testsuite/cmd-0r.sh			\
  testsuite/cmd-R.sh			\
  testsuite/colon-with-no-label.sh	\
  testsuite/comment-n.sh		\
  testsuite/compile-errors.sh		\
  testsuite/compile-tests.sh		\
  testsuite/convert-number.sh		\
  testsuite/command-endings.sh		\
  testsuite/debug.pl			\
  testsuite/execute-tests.sh		\
  testsuite/help-version.sh		\
  testsuite/in-place-hyphen.sh		\
  testsuite/in-place-suffix-backup.sh	\
  testsuite/inplace-selinux.sh		\
  testsuite/invalid-mb-seq-UMR.sh	\
  testsuite/mb-bad-delim.sh		\
  testsuite/mb-charclass-non-utf8.sh	\
  testsuite/mb-match-slash.sh		\
  testsuite/mb-y-translate.sh		\
  testsuite/missing-filename.sh		\
  testsuite/newline-dfa-bug.sh		\
  testsuite/normalize-text.sh		\
  testsuite/nulldata.sh			\
  testsuite/obinary.sh			\
  testsuite/panic-tests.sh		\
  testsuite/posix-char-class.sh		\
  testsuite/posix-mode-addr.sh		\
  testsuite/posix-mode-bad-ref.sh	\
  testsuite/posix-mode-ERE.sh		\
  testsuite/posix-mode-s.sh		\
  testsuite/posix-mode-N.sh		\
  testsuite/range-overlap.sh		\
  testsuite/recursive-escape-c.sh	\
  testsuite/regex-errors.sh		\
  testsuite/regex-max-int.sh		\
  testsuite/sandbox.sh			\
  testsuite/stdin-prog.sh		\
  testsuite/subst-options.sh		\
  testsuite/subst-mb-incomplete.sh	\
  testsuite/subst-replacement.sh	\
  testsuite/temp-file-cleanup.sh	\
  testsuite/title-case.sh		\
  testsuite/unbuffered.sh

if TEST_SYMLINKS
T += testsuite/follow-symlinks.sh		\
     testsuite/follow-symlinks-stdin.sh
endif

# Old tests converted to newer init.sh style
T += testsuite/8bit.sh			\
     testsuite/8to7.sh			\
     testsuite/badenc.sh		\
     testsuite/binary.sh		\
     testsuite/bsd-wrapper.sh		\
     testsuite/dc.sh			\
     testsuite/distrib.sh               \
     testsuite/eval.sh			\
     testsuite/help.sh			\
     testsuite/inplace-hold.sh          \
     testsuite/mac-mf.sh		\
     testsuite/madding.sh		\
     testsuite/newjis.sh		\
     testsuite/stdin.sh                 \
     testsuite/utf8-ru.sh		\
     testsuite/uniq.sh			\
     testsuite/word-delim.sh		\
     testsuite/xemacs.sh

TESTS = $(SEDTESTS) $(T)

SEDTESTS =

check_PROGRAMS = testsuite/get-mb-cur-max testsuite/test-mbrtowc
testsuite_get_mb_cur_max_LDADD = lib/libsed.a $(INTLLIBS)
testsuite_test_mbrtowc_LDADD = lib/libsed.a $(INTLLIBS)

# Note that the first lines are statements.  They ensure that environment
# variables that can perturb tests are unset or set to expected values.
# The rest are envvar settings that propagate build-related Makefile
# variables to test scripts.
TESTS_ENVIRONMENT =				\
  tmp__=$${TMPDIR-/tmp};			\
  test -d "$$tmp__" && test -w "$$tmp__" || tmp__=.;	\
  . $(srcdir)/testsuite/envvar-check;		\
  TMPDIR=$$tmp__; export TMPDIR;		\
						\
  if test -n "$$BASH_VERSION" || (eval "export v=x") 2>/dev/null; then \
    export_with_values () { export "$$@"; };		\
  else							\
    export_with_values ()				\
    {							\
      sed_extract_var='s/=.*//';			\
      sed_quote_value="s/'/'\\\\''/g;s/=\\(.*\\)/='\\1'/";\
      for arg in "$$@"; do				\
        var=`echo "$$arg" | sed "$$sed_extract_var"`;	\
        arg=`echo "$$arg" | sed "$$sed_quote_value"`;	\
        eval "$$arg";					\
        export "$$var";					\
      done;						\
    };							\
  fi;							\
						\
  export_with_values				\
  VERSION='$(VERSION)'				\
  LOCALE_FR='$(LOCALE_FR)'			\
  LOCALE_FR_UTF8='$(LOCALE_FR_UTF8)'		\
  LOCALE_JA='$(LOCALE_JA)'			\
  AWK=$(AWK)					\
  LC_ALL=C					\
  abs_top_builddir='$(abs_top_builddir)'	\
  abs_top_srcdir='$(abs_top_srcdir)'		\
  abs_srcdir='$(abs_srcdir)'			\
  built_programs=sed				\
  srcdir='$(srcdir)'				\
  top_srcdir='$(top_srcdir)'			\
  CC='$(CC)'					\
  CONFIG_HEADER='$(CONFIG_HEADER)'		\
  SED_TEST_NAME=`echo $$tst|sed 's,^\./,,;s,/,-,g'` \
  MAKE=$(MAKE)					\
  MALLOC_PERTURB_=$(MALLOC_PERTURB_)		\
  PACKAGE_BUGREPORT='$(PACKAGE_BUGREPORT)'	\
  PACKAGE_VERSION=$(PACKAGE_VERSION)		\
  PERL='$(PERL)'				\
  SHELL='$(SHELL)'				\
  PATH='$(abs_top_builddir)/sed$(PATH_SEPARATOR)'"$$PATH" \
  $(LOCALCHARSET_TESTS_ENVIRONMENT)		\
  ; 9>&2

EXTRA_DIST += \
	$(T) \
	testsuite/Coreutils.pm					\
	testsuite/CuSkip.pm					\
	testsuite/CuTmpdir.pm					\
	testsuite/init.sh init.cfg \
	testsuite/envvar-check \
	testsuite/8bit.good \
	testsuite/8bit.inp \
	testsuite/binary.sed \
	testsuite/binary2.sed \
	testsuite/binary3.sed \
	testsuite/bsd.good \
	testsuite/bsd.sh \
	testsuite/dc.sed \
	testsuite/distrib.inp \
	testsuite/mac-mf.good \
	testsuite/mac-mf.inp \
	testsuite/mac-mf.sed \
	testsuite/madding.good \
	testsuite/madding.inp \
	testsuite/madding.sed \
	testsuite/no-perl \
	testsuite/uniq.good \
	testsuite/uniq.inp \
	testsuite/uniq.sed \
	testsuite/xemacs.good \
	testsuite/xemacs.inp

# automake makes `check' depend on $(TESTS).  Declare
# dummy targets for $(TESTS) so that make does not complain.

.PHONY: $(SEDTESTS)
$(SEDTESTS):
