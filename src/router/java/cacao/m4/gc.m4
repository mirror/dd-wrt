dnl m4/gc.m4
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


dnl check for garbage collector usage

AC_DEFUN([AC_CHECK_ENABLE_GC],[
AC_ARG_ENABLE([gc], [AS_HELP_STRING(--enable-gc,enable garbage collector support (none,boehm,cacao) [[default=boehm]])])
AC_MSG_CHECKING(whether GC should be enabled)
enable_gc=${enable_gc:-boehm}
case "$enable_gc" in
no | none)
    AC_DEFINE([DISABLE_GC], 1, [disable garbage collector])
    ENABLE_GC=none
    AC_MSG_RESULT(no)
    ;;

boehm)
    AC_DEFINE([ENABLE_GC_BOEHM], 1, [enable conservative boehm-gc])
    ENABLE_GC=boehm
    AC_MSG_RESULT(yes, boehm-gc)
    ;;

cacao)
    AC_DEFINE([ENABLE_GC_CACAO], 1, [enable exact cacao-gc])
    ENABLE_GC=cacao
    AC_MSG_RESULT(yes, cacao-gc)
    ;;

*)
    AC_MSG_ERROR($enable_gc is an unknown garbage collector package)
    ;;
esac
AM_CONDITIONAL([DISABLE_GC], test x"${ENABLE_GC}" = "xnone")
AM_CONDITIONAL([ENABLE_GC_BOEHM], test x"${ENABLE_GC}" = "xboehm")
AM_CONDITIONAL([ENABLE_GC_CACAO], test x"${ENABLE_GC}" = "xcacao")
])
