dnl m4/soft.m4
dnl
dnl Copyright (C) 1996-2005, 2006, 2007 R. Grafl, A. Krall, C. Kruegel,
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


dnl check if softfloat should be used

AC_DEFUN([AC_CHECK_SOFTFLOAT],[
AC_MSG_CHECKING(whether softfloat should be used)
AC_ARG_ENABLE([softfloat],
              [AS_HELP_STRING(--enable-softfloat,use softfloat [[default=no]])],
              [case "${enableval}" in
                  yes)
                      ENABLE_SOFTFLOAT=yes
                      ;;
                  *)
                      ENABLE_SOFTFLOAT=no
                      ;;
               esac],
              [ENABLE_SOFTFLOAT=no])
AC_MSG_RESULT(${ENABLE_SOFTFLOAT})

if test x"${ENABLE_SOFTFLOAT}" = "xyes"; then
    AC_DEFINE([ENABLE_SOFTFLOAT], 1, [enable softfloat])
fi
])


dnl check if soft float compare should be used

AC_DEFUN([AC_CHECK_SOFT_FLOAT_CMP],[
AC_MSG_CHECKING(whether soft float cmp should be used)
AC_ARG_ENABLE([soft-float-cmp],
              [AS_HELP_STRING(--enable-soft-float-cmp,use soft float cmp [[default=no]])],
              [case "${enableval}" in
                  yes)
                      ENABLE_SOFT_FLOAT_CMP=yes
                      ;;
                  *)
                      ENABLE_SOFT_FLOAT_CMP=no
                      ;;
               esac],
              [ENABLE_SOFT_FLOAT_CMP=no])
AC_MSG_RESULT(${ENABLE_SOFT_FLOAT_CMP})

if test x"${ENABLE_SOFT_FLOAT_CMP}" = "xyes"; then
    AC_DEFINE([ENABLE_SOFT_FLOAT_CMP], 1, [enable soft float cmp])
fi
])


dnl check if soft double compare should be used

AC_DEFUN([AC_CHECK_SOFT_DOUBLE_CMP],[
AC_MSG_CHECKING(whether soft double cmp should be used)
AC_ARG_ENABLE([soft-double-cmp],
              [AS_HELP_STRING(--enable-soft-double-cmp,use soft double cmp [[default=no]])],
              [case "${enableval}" in
                  yes)
                      ENABLE_SOFT_DOUBLE_CMP=yes
                      ;;
                  *)
                      ENABLE_SOFT_DOUBLE_CMP=no
                      ;;
               esac],
              [ENABLE_SOFT_DOUBLE_CMP=no])
AC_MSG_RESULT(${ENABLE_SOFT_DOUBLE_CMP})

if test x"${ENABLE_SOFT_DOUBLE_CMP}" = "xyes"; then
    AC_DEFINE([ENABLE_SOFT_DOUBLE_CMP], 1, [enable soft double cmp])
fi
])
