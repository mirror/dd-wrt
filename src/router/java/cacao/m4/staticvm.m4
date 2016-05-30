dnl m4/staticvm.m4
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


dnl check if CACAO should be linked statically

AC_DEFUN([AC_CHECK_ENABLE_STATICVM],[
AC_MSG_CHECKING(whether to link CACAO statically)
AC_ARG_ENABLE([staticvm],
              [AS_HELP_STRING(--enable-staticvm,link CACAO statically [[default=no]])],
              [case "${enableval}" in
                  yes)
                      ENABLE_STATICVM=yes
                      AC_DEFINE([ENABLE_STATICVM], 1, [link CACAO statically])
                      ;;
                  *)
                      ENABLE_STATICVM=no
                      ;;
               esac],
              [ENABLE_STATICVM=no])
AC_MSG_RESULT(${ENABLE_STATICVM})
AM_CONDITIONAL([ENABLE_STATICVM], test x"${ENABLE_STATICVM}" = "xyes")
AC_SUBST(ENABLE_STATICVM)
])
