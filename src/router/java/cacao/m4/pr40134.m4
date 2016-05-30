dnl m4/pr40134.m4
dnl
dnl Copyright (C) 2010
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


AC_DEFUN([AC_CHECK_ENABLE_GCC_PR40134],[
AC_MSG_CHECKING([for gcc PR40134 workaround])
AC_ARG_ENABLE([pr40134-workaround],
              [AS_HELP_STRING(--enable-pr40134-workaround,[enable workaround for gcc PR40134, may be required on ARM] [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_PR40134=yes;;
                   *) ENABLE_PR40134=no;;
               esac],
              [ENABLE_PR40134=no])
AC_MSG_RESULT(${ENABLE_PR40134})
AM_CONDITIONAL([GCC_PR40134], [test x"${ENABLE_PR40134}" = "xyes"])
])
