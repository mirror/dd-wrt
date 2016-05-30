dnl m4/libjvm.m4
dnl
dnl Copyright (C) 2007 R. Grafl, A. Krall, C. Kruegel,
dnl C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
dnl E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
dnl J. Wenninger, Institut f. Computersprachen - TU Wien
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


dnl check if a libjvm.so should be built

AC_DEFUN([AC_CHECK_ENABLE_LIBJVM],[
AC_MSG_CHECKING(whether to build a libjvm.so)
AC_ARG_ENABLE([libjvm],
              [AS_HELP_STRING(--disable-libjvm,build a libjvm.so [[default=enabled]])],
              [case "${enableval}" in
                  no)
                      ENABLE_LIBJVM=no
                      ;;
                  *)
                      ENABLE_LIBJVM=yes
                      ;;
               esac],
              [ENABLE_LIBJVM=yes])
AC_MSG_RESULT(${ENABLE_LIBJVM})
AM_CONDITIONAL([ENABLE_LIBJVM], test x"${ENABLE_LIBJVM}" = "xyes")
AC_SUBST(ENABLE_LIBJVM)

if test x"${ENABLE_LIBJVM}" = "xyes"; then
    AC_DEFINE([ENABLE_LIBJVM], 1, [enable libjvm.so])

    dnl set AC_ENABLE_SHARED and AC_DISABLE_STATIC properly
    enable_shared=yes
    enable_static=no
else
    enable_shared=no
    enable_static=yes
fi
])
