dnl m4/opagent.m4
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

dnl check if opagent library should be used

AC_DEFUN([AC_CHECK_ENABLE_OPAGENT],[
AC_ARG_WITH([opagent-includedir],
            [AS_HELP_STRING(--with-opagent-includedir=<path>,location of opagent header files [[default=/usr/include]])],
            [OPAGENT_CPPFLAGS=-I"${withval}"],
            [])

AC_ARG_WITH([opagent-libdir],
            [AS_HELP_STRING(--with-opagent-libdir=<path>,location of opagent library [[default=/usr/lib/oprofile]])],
            [OPAGENT_LDFLAGS=-L"${withval}"],
            [OPAGENT_LDFLAGS=-L/usr/lib/oprofile])

AC_MSG_CHECKING(whether Oprofile opagent support code should be compiled)
AC_ARG_ENABLE([opagent],
              [AS_HELP_STRING(--enable-opagent,enable opagent library support [[default=disabled]])],
              [case "${enableval}" in
                  yes) ENABLE_OPAGENT=yes;;
                  *) ENABLE_OPAGENT=no;;
               esac],
              [ENABLE_OPAGENT=no])
AC_MSG_RESULT(${ENABLE_OPAGENT})

if test x"${ENABLE_OPAGENT}" = "xyes"; then
    AM_CPPFLAGS="$AM_CPPFLAGS $OPAGENT_CPPFLAGS"
    AM_LDFLAGS="$AM_LDFLAGS $OPAGENT_LDFLAGS"
    AC_CHECK_HEADERS([opagent.h],, [AC_MSG_ERROR(cannot find opagent.h)])
    AC_CHECK_LIB(opagent, op_open_agent,, [AC_MSG_ERROR(cannot find libopagent)])
    AC_DEFINE([ENABLE_OPAGENT], 1, [use opagent])
fi
AM_CONDITIONAL([ENABLE_OPAGENT], [test x"${ENABLE_OPAGENT}" = "xyes"])
])

