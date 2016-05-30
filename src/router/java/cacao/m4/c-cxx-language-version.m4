dnl Copyright (C) 1996-2014
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


dnl Determine C/C++ language version to use

AC_DEFUN([AC_CHECK_WITH_CXX_VERSION],[
AC_MSG_CHECKING(which C++ language version to use)
AC_ARG_WITH([cxx-standard],
            [AS_HELP_STRING(--with-cxx-standard=<version>, specifies which version of the C++ language to use (98,11) [[default=98]])],
            [case "${withval}" in
                98)
                    CXX_LANGUAGE_VERSION=c++98
                    ;;
                11)
                    CXX_LANGUAGE_VERSION=c++11
                    ;;
                *)
                    AC_MSG_ERROR(unknown C++ version ${withval})
                    ;;
             esac],
            [CXX_LANGUAGE_VERSION=c++98])
AC_MSG_RESULT(${CXX_LANGUAGE_VERSION})
])


AC_DEFUN([AC_CHECK_WITH_C_VERSION],[
AC_MSG_CHECKING(which C language version to use)
AC_ARG_WITH([c-standard],
            [AS_HELP_STRING(--with-c-standard=<version>, specifies which version of the C language to use (99,11) [[default=99]])],
            [case "${withval}" in
                99)
                    C_LANGUAGE_VERSION=c99
                    ;;
                11)
                    C_LANGUAGE_VERSION=c11
                    ;;
                *)
                    AC_MSG_ERROR(unknown C version ${withval})
                    ;;
             esac],
            [C_LANGUAGE_VERSION=c99])
AC_MSG_RESULT(${C_LANGUAGE_VERSION})
])
