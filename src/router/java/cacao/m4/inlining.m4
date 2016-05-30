dnl m4/inlining.m4
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


dnl check for inlining

AC_DEFUN([AC_CHECK_ENABLE_INLINING],[
AC_MSG_CHECKING(whether method inlining should be supported)
AC_ARG_ENABLE([inlining],
              [AS_HELP_STRING(--enable-inlining,enable method inlining [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_INLINING=yes;;
                   *) ENABLE_INLINING=no;;
               esac],
              [ENABLE_INLINING=no])
AC_MSG_RESULT(${ENABLE_INLINING})
AM_CONDITIONAL([ENABLE_INLINING], test x"${ENABLE_INLINING}" = "xyes")

if test x"${ENABLE_INLINING}" = "xyes"; then
    AC_DEFINE([ENABLE_INLINING], 1, [use method inlining])
fi
])


dnl check for inlining debug options

AC_DEFUN([AC_CHECK_ENABLE_INLINING_DEBUG],[
AC_MSG_CHECKING(whether method inlining debug options should be enabled)
AC_ARG_ENABLE([inlining-debug],
              [AS_HELP_STRING(--enable-inlining-debug,enable method inlining debug options [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_INLINING_DEBUG=yes;;
                   *) ENABLE_INLINING_DEBUG=no;;
               esac],
              [ENABLE_INLINING_DEBUG=no])
AC_MSG_RESULT(${ENABLE_INLINING_DEBUG})
AM_CONDITIONAL([ENABLE_INLINING_DEBUG], test x"${ENABLE_INLINING_DEBUG}" = "xyes")

if test x"${ENABLE_INLINING_DEBUG}" = "xyes"; then
    AC_DEFINE([ENABLE_INLINING_DEBUG], 1, [enable method inlining debug options])
fi
])
