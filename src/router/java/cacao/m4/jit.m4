dnl m4/jit.m4
dnl
dnl Copyright (C) 2007 R. Grafl, A. Krall, C. Kruegel,
dnl C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
dnl E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
dnl J. Wenninger, Institut f. Computersprachen - TU Wien
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


dnl check for JIT compiler

AC_DEFUN([AC_CHECK_ENABLE_JIT],[
AC_MSG_CHECKING(whether JIT compiler should be compiled)
AC_ARG_ENABLE([jit],
              [AS_HELP_STRING(--disable-jit,disable JIT compiler [[default=enabled]])],
              [case "${enableval}" in
                   no)
                       ENABLE_JIT=no
                       ;;
                   *)
                       ENABLE_JIT=yes
                       ;;
               esac],
              [ENABLE_JIT=yes])
AC_MSG_RESULT(${ENABLE_JIT})
AM_CONDITIONAL([ENABLE_JIT], test x"${ENABLE_JIT}" = "xyes")

if test x"${ENABLE_JIT}" = "xyes"; then
    AC_DEFINE([ENABLE_JIT], 1, [enable JIT compiler])
fi
])
