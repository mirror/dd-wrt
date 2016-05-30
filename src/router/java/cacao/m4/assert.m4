dnl m4/assert.m4
dnl
dnl Copyright (C) 1996-2013
dnl CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
dnl 
dnl This file is part of CACAO.
dnl 
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2, or (at
dnl your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.

dnl Advanced C++ assertion support

dnl check if C++11 builtin static assertions are available

AC_DEFUN([AC_CHECK_HAVE_BUILTIN_STATIC_ASSERT],[
  AC_MSG_CHECKING(whether static_assert is available)
  AC_LANG_PUSH([C++])

  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[ ]], [[static_assert(check_type::value >= 201103L, "C++11 compiler");]])],
      [AC_DEFINE([HAVE_BUILTIN_STATIC_ASSERT], 1, [C++11 static_assert is available])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)]
  )

  AC_LANG_POP([C++])
])

dnl check if EXPENSIVE_ASSERT should be enabled

AC_DEFUN([AC_CHECK_ENABLE_EXPENSIVE_ASSERT],[
AC_MSG_CHECKING(whether EXPENSIVE_ASSERT should be enabled)
AC_ARG_ENABLE([expensive-assert],
              [AS_HELP_STRING(--enable-expensive-assert,enable expensive assertions [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_EXPENSIVE_ASSERT=yes;;
                   *) ENABLE_EXPENSIVE_ASSERT=no;;
               esac],
              [ENABLE_EXPENSIVE_ASSERT=no])
AC_MSG_RESULT(${ENABLE_EXPENSIVE_ASSERT})
AM_CONDITIONAL([ENABLE_EXPENSIVE_ASSERT], test x"${ENABLE_EXPENSIVE_ASSERT}" = "xyes")

if test x"${ENABLE_EXPENSIVE_ASSERT}" = "xyes"; then
    AC_DEFINE([ENABLE_EXPENSIVE_ASSERT], 1, [enable expensive assertions])
fi
])
