dnl m4/jvmti.m4
dnl
dnl Copyright (C) 2008, 2009
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


dnl check for JVMTI

AC_DEFUN([AC_CHECK_ENABLE_JVMTI],[
AC_MSG_CHECKING(whether to compile JVMTI support)
AC_ARG_ENABLE([jvmti],
              [AS_HELP_STRING(--enable-jvmti,enable JVMTI [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_JVMTI=yes;;
                   *) ENABLE_JVMTI=no;;
               esac],
              [ENABLE_JVMTI=no])
AC_MSG_RESULT(${ENABLE_JVMTI})
AM_CONDITIONAL([ENABLE_JVMTI], test x"${ENABLE_JVMTI}" = "xyes")

if test x"${ENABLE_JVMTI}" = "xyes"; then
    AC_DEFINE([ENABLE_JVMTI], 1, [use JVMTI])
fi
])


dnl where jvmti.h is installed

AC_DEFUN([AC_CHECK_WITH_JVMTI_H],[
AC_MSG_CHECKING(where jvmti.h is installed)
AC_ARG_WITH([jvmti_h],
            [AS_HELP_STRING(--with-jvmti_h=<dir>,path to jvmti.h (only with --enable-jvmti) [[default=(openjdk:${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export,*:${JAVA_RUNTIME_LIBRARY_PREFIX}/include)]])],
            [WITH_JVMTI_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk)
                     WITH_JVMTI_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export
                     ;;
                 *)
                     WITH_JVMTI_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/include
                     ;;
            esac])
AC_MSG_RESULT(${WITH_JVMTI_H})

AC_CHECK_HEADER([${WITH_JVMTI_H}/jvmti.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_JVMTI_H], "${WITH_JVMTI_H}/jvmti.h", [Java runtime library jvmti.h header])],
                [AC_MSG_ERROR(cannot find jvmti.h)])
])
