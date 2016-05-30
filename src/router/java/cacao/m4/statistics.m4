dnl m4/statistics.m4
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


dnl check for statistics

AC_DEFUN([AC_CHECK_ENABLE_STATISTICS],[
AC_MSG_CHECKING(whether statistics generation should be enabled)
AC_ARG_ENABLE([statistics],
              [AS_HELP_STRING(--enable-statistics,enable statistics generation [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_STATISTICS=yes;;
                   *) ENABLE_STATISTICS=no;;
               esac],
              [ENABLE_STATISTICS=no])
AC_MSG_RESULT(${ENABLE_STATISTICS})
AM_CONDITIONAL([ENABLE_STATISTICS], test x"${ENABLE_STATISTICS}" = "xyes")

if test x"${ENABLE_STATISTICS}" = "xyes"; then
    AC_DEFINE([ENABLE_STATISTICS], 1, [enable statistics])
fi
])
