dnl m4/intrp.m4
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


dnl check for interpreter

AC_DEFUN([AC_CHECK_ENABLE_INTRP],[
AC_ARG_ENABLE([intrp], [AS_HELP_STRING(--enable-intrp,enable interpreter [[default=no]])])

AC_MSG_CHECKING(whether interpreter should be compiled)
if test x"$enable_intrp" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE([ENABLE_INTRP], 1, [enable interpreter])
    AM_CONDITIONAL([ENABLE_INTRP], [true])
    ENABLE_INTRP=yes

    AC_ASM_SKIP16

    dnl check for libffi
    AC_ARG_WITH([ffi],
                [AS_HELP_STRING(--with-ffi,use libffi to call native functions)],
                [WITH_FFI=yes
                 AC_CHECK_HEADERS([ffi.h],, [AC_MSG_ERROR(cannot find ffi.h)])
                 AC_CHECK_LIB(ffi, ffi_call,, [AC_MSG_ERROR(cannot find libffi)])
                 AC_DEFINE([WITH_FFI], 1, [use libffi])],
                [WITH_FFI=no])

    dnl check for libffcall
    AC_ARG_WITH([ffcall],
                [AS_HELP_STRING(--with-ffcall,use libffcall to call native functions)],
                [WITH_FFCALL=yes
                 AC_CHECK_HEADERS([avcall.h],, [AC_MSG_ERROR(cannot find avcall.h)])
                 AC_CHECK_LIB(avcall, __builtin_avcall,, [AC_MSG_ERROR(cannot find libavcall (from libffcall))])
                 AC_DEFINE([WITH_FFCALL], 1, [use libffcall])],
                [WITH_FFCALL=no])

    dnl check for libffi or libffcall
    if test x"${WITH_FFI}" = "xyes" -a x"${WITH_FFCALL}" = "xyes"; then
        AC_ERROR(Enable only libffi or libffcall.)
    fi
    if test x"${WITH_FFI}" = "xno" -a x"${WITH_FFCALL}" = "xno"; then
        AC_ERROR(Enable one of libffi or libffcall.)
    fi

else
    AC_MSG_RESULT(no)
    AM_CONDITIONAL([ENABLE_INTRP], [false])
    ENABLE_INTRP="no"
fi
])
