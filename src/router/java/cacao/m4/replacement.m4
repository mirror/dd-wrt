dnl m4/replacement.m4
dnl
dnl Copyright (C) 2008
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


dnl check for replacement

AC_DEFUN([AC_CHECK_ENABLE_REPLACEMENT],[
AC_MSG_CHECKING(whether on-stack replacement should be supported)
AC_ARG_ENABLE([replacement],
              [AS_HELP_STRING(--enable-replacement,enable on-stack replacement [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_REPLACEMENT=yes;;
                   *) ENABLE_REPLACEMENT=no;;
               esac],
              [ENABLE_REPLACEMENT=no])
AC_MSG_RESULT(${ENABLE_REPLACEMENT})
AM_CONDITIONAL([ENABLE_REPLACEMENT], test x"${ENABLE_REPLACEMENT}" = "xyes")

if test x"${ENABLE_REPLACEMENT}" = "xyes"; then
    AC_DEFINE([ENABLE_REPLACEMENT], 1, [use on-stack replacement])
fi
])
