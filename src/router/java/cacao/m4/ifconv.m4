dnl m4/ifconv.m4
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


dnl check if if-conversion should be supported

AC_DEFUN([AC_CHECK_ENABLE_IFCONV],[
AC_MSG_CHECKING(whether if-conversion should be supported)
AC_ARG_ENABLE([ifconv],
              [AS_HELP_STRING(--disable-ifconv,disable if-conversion [[default=enabled]])],
              [case "${enableval}" in
                   no) ENABLE_IFCONV=no;;
                   *) ENABLE_IFCONV=yes;;
               esac],
              [ENABLE_IFCONV=yes])
AC_MSG_RESULT(${ENABLE_IFCONV})
AM_CONDITIONAL([ENABLE_IFCONV], test x"${ENABLE_IFCONV}" = "xyes")
   
if test x"${ENABLE_IFCONV}" = "xyes"; then
    AC_DEFINE([ENABLE_IFCONV], 1, [enable if-conversion])
fi
])
