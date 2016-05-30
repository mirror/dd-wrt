dnl m4/logging.m4
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

dnl check for logging

AC_DEFUN([AC_CHECK_ENABLE_LOGGING],[
AC_MSG_CHECKING(whether logging should be enabled)
AC_ARG_ENABLE([logging],
              [AS_HELP_STRING(--enable-logging,enable logging [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_LOGGING=yes;;
                   *) ENABLE_LOGGING=no;;
               esac],
              [ENABLE_LOGGING=no])
AC_MSG_RESULT(${ENABLE_LOGGING})
AM_CONDITIONAL([ENABLE_LOGGING], test x"${ENABLE_LOGGING}" = "xyes")

if test x"${ENABLE_LOGGING}" = "xyes"; then
    AC_DEFINE([ENABLE_LOGGING], 1, [enable logging])
fi
])
