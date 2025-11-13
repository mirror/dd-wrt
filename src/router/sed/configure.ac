# Copyright (C) 1993-2022 Free Software Foundation, Inc.

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

dnl Process this file with -*- autoconf -*- to produce a configure script.
AC_INIT([GNU sed],
        m4_esyscmd([build-aux/git-version-gen .tarball-version]),
        [bug-sed@gnu.org])
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_SRCDIR([sed/sed.c])
AC_CONFIG_HEADERS([config.h:config_h.in])
AC_PREREQ([2.64])
AM_INIT_AUTOMAKE([1.11.1 dist-xz color-tests parallel-tests subdir-objects])
AM_SILENT_RULES([yes]) # make --enable-silent-rules the default.
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_CC_C_O
gl_EARLY
gl_INIT
gl_DISABLE_THREADS

# Ensure VLAs are not used.
# Note -Wvla is implicitly added by gl_MANYWARN_ALL_GCC
AC_DEFINE([GNULIB_NO_VLA], [1], [Define to 1 to disable use of VLAs])

# The test suite needs to know if we have a working perl.
AM_CONDITIONAL([HAVE_PERL], [test "$gl_cv_prog_perl" != no])

# gl_GCC_VERSION_IFELSE([major], [minor], [run-if-found], [run-if-not-found])
# ------------------------------------------------
# If $CPP is gcc-MAJOR.MINOR or newer, then run RUN-IF-FOUND.
# Otherwise, run RUN-IF-NOT-FOUND.
AC_DEFUN([gl_GCC_VERSION_IFELSE],
  [AC_PREPROC_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#if ($1) < __GNUC__ || (($1) == __GNUC__ && ($2) <= __GNUC_MINOR__)
/* ok */
#else
# error "your version of gcc is older than $1.$2"
#endif
      ]]),
    ], [$3], [$4])
  ]
)

AC_CACHE_CHECK([whether "rt" can be used with fopen], [sed_cv_fopen_rt], [
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([[
#include <stdio.h>
#include <errno.h>

int main()
{
  FILE *fp;
  int result;
  errno = 0;
  fp = fopen ("conftest.c", "rt");
  if (fp) fclose (fp);
  return fp ? 0 : 1;
}]])], [sed_cv_fopen_rt=yes],
    [sed_cv_fopen_rt=no],
    [case $host in
       *cygwin* | *mingw*) sed_cv_fopen_rt=yes ;;
       *) sed_cv_fopen_rt='assuming no' ;;
     esac])])
if test "$sed_cv_fopen_rt" = yes; then
  AC_DEFINE([HAVE_FOPEN_RT], [1],
            [Defined if "rt" can be used as a mode to fopen.])
fi

AC_CACHE_CHECK([whether -lcP is needed], [sed_cv_libcp_needed], [
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([[
#include <stdio.h>
#include <errno.h>

int main()
{
  FILE *fp;
  int result;
  errno = 0;
  fp = fopen ("conftest.c", "r");
  if (!fp) return 0;	   /* error, assume not needed */
  result = fflush (fp) == EOF && errno == 0;
  fclose (fp);
  return result;
}]])], [sed_cv_libcp_needed=no],
    [sed_cv_libcp_needed=yes],
    [sed_cv_libcp_needed="assuming no"])
])
if test "$sed_cv_libcp_needed" = yes; then
  LIBS="-lcP $LIBS"
fi

AC_CHECK_HEADERS_ONCE(locale.h errno.h wchar.h wctype.h mcheck.h,
                      [], [], [AC_INCLUDES_DEFAULT])
AC_C_CONST
AC_TYPE_SIZE_T

AM_GNU_GETTEXT_VERSION([0.19.2])
AM_GNU_GETTEXT([external])

AC_CHECK_FUNCS_ONCE(isatty isascii memcpy strchr strtoul readlink
                    popen pathconf fchown fchmod setlocale)

AM_CONDITIONAL([TEST_SYMLINKS],
          [test "$ac_cv_func_readlink" = yes])

AC_ARG_ENABLE(i18n,
[  --disable-i18n          disable internationalization (default=enabled)], ,
enable_i18n=yes)
if test "x$enable_i18n" = xno; then
  ac_cv_func_wcscoll=no
fi


# Determine whether we should run UTF-8 tests by checking if cyrillic
# letters are case-folded properly.  The test for UTF-8 locales (both
# in general and specifically for a Russian one) is a bit weak, but it
# should match exactly what is done in the testsuite.  In fact, sed's
# logic is portable (though testing it requires care) so it is enough to
# have a couple of platforms where these tests pass.  Right now, only
# Windows and HP/UX do not support the tests.
AC_MSG_CHECKING([whether UTF-8 case folding tests should pass])
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([[
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif

int test(void)
{
  char in[] = "\xD0\xB4";
  char good[] = "\xD0\x94";
  char out[10];
  wchar_t in_wc, good_wc;
  if (mbtowc (&in_wc, in, 3) == -1)
    return 0;
  if (mbtowc (&good_wc, good, 3) == -1)
    return 0;
  if (towupper (in_wc) != good_wc)
    return 0;
  if (wctomb (out, good_wc) != 2)
    return 0;
  if (memcmp (out, good, 2))
    return 0;
  return 1;
}

int main()
{
  char *old;
  int len;

  /* Try hardcoding a Russian UTF-8 locale.  If the name "ru_RU.UTF-8"
     is invalid, use setlocale again just to get the current locale.  */
  old = setlocale (LC_CTYPE, "ru_RU.UTF-8");
  if (old)
    {
      if (test())
        exit (0);
    }
  else
    old = setlocale (LC_CTYPE, "C");

  /* Maybe cyrillic case folding is implemented for all UTF-8 locales.
     If the current locale is not UTF-8, the test will be skipped.  */
  len = strlen (old);
  if ((len > 6 && !strcmp (old + len - 6, ".UTF-8"))
      || (len > 6 && !strcmp (old + len - 6, ".utf-8"))
      || (len > 5 && !strcmp (old + len - 5, ".UTF8"))
      || (len > 5 && !strcmp (old + len - 5, ".utf8")))

    /* ok */
    ;
  else
    exit (1);

  /* Run the test in the detected UTF-8 locale.  */
  setlocale (LC_CTYPE, old);
  exit (!test ());
}
]])], [AC_MSG_RESULT([yes]); XFAIL_TESTS=],
   [AC_MSG_RESULT([no]); XFAIL_TESTS='testsuite/utf8-1 testsuite/utf8-2 \
                                      testsuite/utf8-3 testsuite/utf8-4'],
   [AC_MSG_RESULT([don't care (cross compiling)]); XFAIL_TESTS=])

# Under MinGW, the bsd.sh test fails because of the EOF character (^Z).
case $host in
  *mingw*) XFAIL_TESTS="$XFAIL_TESTS bsd" ;;
  *) ;;
esac
AC_SUBST([XFAIL_TESTS])

AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
     [turn on many GCC warnings (for developers; best with GNU make)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [
   # GCC provides fine-grained control over diagnostics which
   # is used in gnulib for example to suppress warnings from
   # certain sections of code.  So if this is available and
   # we're running from a git repo, then auto enable the warnings.
   gl_gcc_warnings=no
   gl_GCC_VERSION_IFELSE([4], [6],
                         [test -d "$srcdir"/.git \
                          && ! test -f "$srcdir"/.tarball-version \
                          && gl_gcc_warnings=yes])]
)

if test "$gl_gcc_warnings" = yes; then
  gl_WARN_ADD([-Werror], [WERROR_CFLAGS])
  AC_SUBST([WERROR_CFLAGS])

  nw=
  # This, $nw, is the list of warnings we disable.
  nw="$nw -Wdeclaration-after-statement" # too useful to forbid
  nw="$nw -Wsystem-headers"         # Don't let system headers trigger warnings
  nw="$nw -Wsign-conversion"        # Too many warnings for now
  nw="$nw -Wformat-nonliteral"      # who.c and pinky.c strftime uses
  nw="$nw -Wswitch-default"         # Too many warnings for now

  gl_MANYWARN_ALL_GCC([ws])
  gl_MANYWARN_COMPLEMENT([ws], [$ws], [$nw])
  for w in $ws; do
    gl_WARN_ADD([$w])
  done
  gl_WARN_ADD([-Wno-sign-compare])     # Too many warnings for now

  # clang is unduly picky about some things.
  AC_CACHE_CHECK([whether the compiler is clang], [utils_cv_clang],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([[
            #ifndef __clang__
              #error "not clang"
            #endif
          ]])],
       [utils_cv_clang=yes],
       [utils_cv_clang=no])])
  if test $utils_cv_clang = yes; then
    gl_WARN_ADD([-Wno-format-extra-args])
    gl_WARN_ADD([-Wno-tautological-constant-out-of-range-compare])
  fi

  gl_WARN_ADD([-fdiagnostics-show-option])
  gl_WARN_ADD([-funit-at-a-time])

  AC_SUBST([WARN_CFLAGS])

  AC_DEFINE([lint], [1], [Define to 1 if the compiler is checking for lint.])
  AH_VERBATIM([FORTIFY_SOURCE],
  [/* Enable compile-time and run-time bounds-checking, and some warnings,
      without upsetting glibc 2.15+. */
   #if !defined _FORTIFY_SOURCE && defined __OPTIMIZE__ && __OPTIMIZE__
   # define _FORTIFY_SOURCE 2
   #endif
  ])
  AC_DEFINE([GNULIB_PORTCHECK], [1], [enable some gnulib portability checks])

  # For gnulib-tests, the set is slightly smaller still.
  nw=
  gl_MANYWARN_COMPLEMENT([GNULIB_TEST_WARN_CFLAGS],
                         [$WARN_CFLAGS], [$nw])
  AC_SUBST([GNULIB_TEST_WARN_CFLAGS])
fi

AC_ARG_ENABLE([bold-man-page-references],
  [AS_HELP_STRING([--disable-bold-man-page-references],
     [When generating man pages, do not apply bold style around any
      references like name(1) etc.])],
  [gl_bold_manpages=yes ;
   case $enableval in
     no|yes) gl_bold_manpages=$enableval ;;
     *)      AC_MSG_ERROR([bad value $enableval for bold-man-page-references.
                           Options are: yes, no.]) ;;
   esac],
  [gl_bold_manpages=yes]
)
AM_CONDITIONAL([BOLD_MAN_REFS], [test "$gl_bold_manpages" != no])

AM_CONDITIONAL([CROSS_COMPILING], [test "$cross_compiling" = yes])

# Perl is needed for help2man
AC_PATH_PROG([PERL], [perl])

# This is needed when building outside the source dir
# with --disable-dependency-tracking, see https://bugs.gnu.org/25371
AS_MKDIR_P([lib])
AS_MKDIR_P([sed])
AS_MKDIR_P([doc])
AS_MKDIR_P([testsuite])

AC_CONFIG_FILES([
  Makefile
  po/Makefile.in
  gnulib-tests/Makefile
])
AC_OUTPUT
