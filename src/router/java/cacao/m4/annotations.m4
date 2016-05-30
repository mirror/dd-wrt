dnl m4/annotations.m4
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


dnl check if annotations support should be built

AC_DEFUN([AC_CHECK_ENABLE_ANNOTATIONS],[
AC_MSG_CHECKING(wether to build annotations support)
AC_ARG_ENABLE([annotations],
              [AS_HELP_STRING(--enable-annotations,build annotations support [[default=(cldc1.1:no,javase:yes]])],
              [case "${enableval}" in
                   yes)
                       ENABLE_ANNOTATIONS=yes
                       ;;
                   no)
                       ENABLE_ANNOTATIONS=no
                       ;;
                   *)
                       AC_CHECK_ENABLE_ANNOTATIONS_DEFAULT
                       ;;
               esac],
              [AC_CHECK_ENABLE_ANNOTATIONS_DEFAULT])
AC_MSG_RESULT(${ENABLE_ANNOTATIONS})
AM_CONDITIONAL([ENABLE_ANNOTATIONS], test x"${ENABLE_ANNOTATIONS}" = "xyes")
   
if test x"${ENABLE_ANNOTATIONS}" = "xyes"; then
    AC_DEFINE([ENABLE_ANNOTATIONS], 1, [enable annotations])
fi
])


dnl check for the default value for --enable-annotations

AC_DEFUN([AC_CHECK_ENABLE_ANNOTATIONS_DEFAULT],[
if test x"${ENABLE_JAVAME_CLDC1_1}" = "xyes"; then
    ENABLE_ANNOTATIONS=no
else
    ENABLE_ANNOTATIONS=yes
fi
])
