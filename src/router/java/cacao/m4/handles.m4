dnl m4/handles.m4
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


dnl check for handles (indirection cells) support

AC_DEFUN([AC_CHECK_ENABLE_HANDLES],[
AC_MSG_CHECKING(whether handles (indirection cells) should be enabled)
AC_ARG_ENABLE([handles],
              [AS_HELP_STRING(--enable-handles,enable handles (indirection cells) [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_HANDLES=yes;;
                   *) ENABLE_HANDLES=no;;
               esac],
              [ENABLE_HANDLES=no])
AC_MSG_RESULT(${ENABLE_HANDLES})
AM_CONDITIONAL([ENABLE_HANDLES], test x"${ENABLE_HANDLES}" = "xyes")

if test x"${ENABLE_HANDLES}" = "xyes"; then
    AC_DEFINE([ENABLE_HANDLES], 1, [enable handles (indirection cells)])
fi
])
