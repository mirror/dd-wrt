dnl aclocal.m4 generated automatically by aclocal 1.4-p6

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

sinclude(../libtool.m4)
dnl The lines below arrange for aclocal not to bring libtool.m4
dnl AC_PROG_LIBTOOL into aclocal.m4, while still arranging for automake
dnl to add a definition of LIBTOOL to Makefile.in.
ifelse(yes,no,[
AC_DEFUN([AC_PROG_LIBTOOL],)
AC_DEFUN([AM_PROG_LIBTOOL],)
AC_SUBST(LIBTOOL)
])

# Newer versions of autoconf add an underscore to these functions.
# Prevent future problems ...
ifdef([AC_PROG_CXX_G],[],[define([AC_PROG_CXX_G],defn([_AC_PROG_CXX_G]))])
ifdef([AC_PROG_CXX_GNU],[],[define([AC_PROG_CXX_GNU],defn([_AC_PROG_CXX_GNU]))])

# We can't just call AC_PROG_CXX directly because g++ will try to
# link in libstdc++.
AC_DEFUN(libdemangler_AC_PROG_CXX,
[AC_BEFORE([$0], [AC_PROG_CXXCPP])dnl
dnl Fool anybody using AC_PROG_CXX.
AC_PROVIDE([AC_PROG_CXX])
AC_CHECK_PROGS(CXX, $CCC c++ g++ gcc CC cxx cc++, gcc)

AC_PROG_CXX_GNU

if test $ac_cv_prog_gxx = yes; then
  GXX=yes
  dnl Check whether -g works, even if CXXFLAGS is set, in case the package
  dnl plays around with CXXFLAGS (such as to build both debugging and
  dnl normal versions of a library), tasteless as that idea is.
  ac_test_CXXFLAGS="${CXXFLAGS+set}"
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS=
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    CXXFLAGS="-g -O2"
  else
    CXXFLAGS="-O2"
  fi
else
  GXX=
  test "${CXXFLAGS+set}" = set || CXXFLAGS="-g"
fi
])

dnl See if we can use the new demangler in C++.
AC_DEFUN(libdemangler_AC_NEW_DEMANGLER,
[AC_CACHE_CHECK(whether we can use the new demangler in C++,
		libdemangler_cv_new_demangler,
[saved_CPPFLAGS="$CPPFLAGS"
 saved_CXXFLAGS="$CXXFLAGS"
 CPPFLAGS="-I$srcdir -I$srcdir/../include -D_GLIBCXX_DEMANGLER_NOSTDCXX $CPPFLAGS"
 CXXFLAGS="-fno-rtti -fno-exceptions $CXXFLAGS"
 libdemangler_cv_new_demangler=no
 ac_compile='${CXX-g++} $CXXFLAGS $CPPFLAGS -c -I$srcdir/include $srcdir/lib/demangle.cc 1>&5'
 if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
   ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS $srcdir/../libiberty/dyn-string.c 1>&5'
   if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
     cat > conftest.c <<EOF
#include <stddef.h>
#include <stdio.h>
#include "demangle.h"

void
xexit (code)
    int code;
{
  exit (code);
}

char *
xmalloc (size)
    size_t size;
{
  return (char *) 0;
}

char *
xrealloc (oldmem, size)
    char *oldmem;
    size_t size;
{
  return (char *) 0;
}

int
main ()
{
  const char* p;
  p = cplus_demangle_v3 ("_Z1fA37_iPS_", 0);
  return !p;
}
EOF
     ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.c 1>&5'
     if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
       ac_link='${CC-cc} -o conftest${ac_exeext} conftest.o demangle.o dyn-string.o $CFLAGS $CPPFLAGS $LDFLAGS $LIBS 1>&5'
       if { (eval echo configure:__oline__: \"$ac_link\") 1>&5; (eval $ac_link ) 2>&5; }; then
         libdemangler_cv_new_demangler=yes
       fi
     fi
   fi
 fi
 rm -f demangle.o dyn-string.o conftest*
 CPPFLAGS="$saved_CPPFLAGS"
 CXXFLAGS="$saved_CXXFLAGS"])
 ])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN([AM_INIT_AUTOMAKE],
[AC_REQUIRE([AM_SET_CURRENT_AUTOMAKE_VERSION])dnl
AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal-${am__api_version}, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake-${am__api_version}, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

# Copyright 2002  Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA

# AM_AUTOMAKE_VERSION(VERSION)
# ----------------------------
# Automake X.Y traces this macro to ensure aclocal.m4 has been
# generated from the m4 files accompanying Automake X.Y.
AC_DEFUN([AM_AUTOMAKE_VERSION],[am__api_version="1.4"])

# AM_SET_CURRENT_AUTOMAKE_VERSION
# -------------------------------
# Call AM_AUTOMAKE_VERSION so it can be traced.
# This function is AC_REQUIREd by AC_INIT_AUTOMAKE.
AC_DEFUN([AM_SET_CURRENT_AUTOMAKE_VERSION],
	 [AM_AUTOMAKE_VERSION([1.4-p6])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN([AM_SANITY_CHECK],
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN([AM_MISSING_PROG],
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN([AM_MAINTAINER_MODE],
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

# Define a conditional.

AC_DEFUN([AM_CONDITIONAL],
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

