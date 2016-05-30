dnl m4/Werror.m4
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


dnl check if we should enable -Werror

AC_DEFUN([AC_CHECK_ENABLE_WERROR],[
AC_MSG_CHECKING(whether to enable -Werror)
AC_ARG_ENABLE([Werror],
              [AS_HELP_STRING(--enable-Werror, enable -Werror compiler flag [[default=no]])],
              [case "${enableval}" in
                   yes) enable_werror=yes;;
                   *) enable_werror=no;;
               esac],
              [enable_werror=no])
AC_MSG_RESULT(${enable_werror})
])
