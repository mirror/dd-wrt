dnl m4/assertion.m4
dnl
dnl Copyright (C) 2007
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


dnl check if java assertion support should be enabled

AC_DEFUN([AC_CHECK_ENABLE_ASSERTION],[
AC_MSG_CHECKING(wether to enable java assertion support)
AC_ARG_ENABLE([assertion],
              [AS_HELP_STRING(--enable-assertion,enable java assertion support [[default=yes]])],
              [case "${enableval}" in
                   yes)
                       ENABLE_ASSERTION=yes
                       ;;
                   *)
                       ENABLE_ASSERTION=no
                       ;;
               esac],
              [ENABLE_ASSERTION=yes])
AC_MSG_RESULT(${ENABLE_ASSERTION})
AM_CONDITIONAL([ENABLE_ASSERTION], test x"${ENABLE_ASSERTION}" = "xyes")
   
if test x"${ENABLE_ASSERTION}" = "xyes"; then
    AC_DEFINE([ENABLE_ASSERTION], 1, [enable assertion])
fi
])
