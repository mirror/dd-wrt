dnl m4/dl.m4
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


dnl check if dynamic library loading should be used

AC_DEFUN([AC_CHECK_ENABLE_DL],[
AC_MSG_CHECKING(whether to support dynamic library loading)
AC_ARG_ENABLE([dl],
              [AS_HELP_STRING(--disable-dl,disable dynamic library loading (needs libdl) [[default=enabled]])],
              [case "${enableval}" in
                  no) ENABLE_DL=no;;
                  *) ENABLE_DL=yes;;
               esac],
              [ENABLE_DL=yes])
AC_MSG_RESULT(${ENABLE_DL})

if test x"${ENABLE_DL}" = "xyes"; then
    AC_CHECK_HEADERS([dlfcn.h],, [AC_MSG_ERROR(cannot find dlfcn.h)])

    case "${OS_DIR}" in
        freebsd | netbsd )
            dnl There is no libdl on FreeBSD, and NetBSD (see PR96).
            ;;
        *)
            AC_CHECK_LIB([dl], [dlopen],, [AC_MSG_ERROR(cannot find libdl)])
            ;;
    esac
    
    AC_CHECK_FUNCS([dlclose])
    AC_CHECK_FUNCS([dlerror])
    AC_CHECK_FUNCS([dlopen])
    AC_CHECK_FUNCS([dlsym])
    AC_DEFINE([ENABLE_DL], 1, [Enable dynamic library loading.])
fi
])
