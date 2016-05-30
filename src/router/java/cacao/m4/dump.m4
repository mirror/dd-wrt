dnl m4/dump.m4
dnl
dnl Copyright (C) 2007, 2008
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


dnl check for dump memory usage

AC_DEFUN([AC_CHECK_ENABLE_DUMP],[
AC_MSG_CHECKING(whether dump memory should be disabled)
AC_ARG_ENABLE([dump],
              [AS_HELP_STRING(--disable-dump,disable dump memory (for debugging only!) [[default=enabled]])],
              [case "${enableval}" in
                   no)
                       DISABLE_DUMP=yes
                       AC_DEFINE([DISABLE_DUMP], 1, [disable dump memory])
                       ;;
                   *)
                       DISABLE_DUMP=no
                       ;;
               esac],
               [DISABLE_DUMP=no])
AC_MSG_RESULT(${DISABLE_DUMP})
AM_CONDITIONAL([DISABLE_DUMP], test x"${DISABLE_DUMP}" = "xno")
])
