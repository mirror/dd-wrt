dnl m4/rt-timing.m4
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


dnl check for compiler timing

AC_DEFUN([AC_CHECK_ENABLE_RT_TIMING],[
AC_MSG_CHECKING(whether real-time timing should be enabled)
AC_ARG_ENABLE([rt-timing],
              [AS_HELP_STRING(--enable-rt-timing,enable real-time timing [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_RT_TIMING=yes;;
                   *) ENABLE_RT_TIMING=no;;
               esac],
              [ENABLE_RT_TIMING=no])
AC_MSG_RESULT(${ENABLE_RT_TIMING})
AM_CONDITIONAL([ENABLE_RT_TIMING], test x"${ENABLE_RT_TIMING}" = "xyes")

if test x"${ENABLE_RT_TIMING}" = "xyes"; then
    AC_DEFINE([ENABLE_RT_TIMING], 1, [enable real-time timing])
    AC_SEARCH_LIBS([clock_gettime], [rt])
fi
])
