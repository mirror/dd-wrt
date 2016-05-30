dnl m4/jvm.m4
dnl
dnl Copyright (C) 1996-2012
dnl CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
dnl Copyright (C) 2008 Theobroma Systems Ltd.
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


dnl Check for jvm_md.h

AC_DEFUN([AC_CHECK_WITH_JVM_MD_H],[
AC_MSG_CHECKING(where jvm_md.h is installed)
AC_ARG_WITH([jvm_md_h],
            [AS_HELP_STRING(--with-jvm_md_h=<dir>,path to jvm_md.h (only with --with-java-runtime-library=openjdk) [[default=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/javavm/export]])],
            [WITH_JVM_MD_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk*)
                     WITH_JVM_MD_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/javavm/export
                     ;;
                 *)
                     ;;
            esac])
AC_MSG_RESULT(${WITH_JVM_MD_H})

AC_CHECK_HEADER([${WITH_JVM_MD_H}/jvm_md.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_JVM_MD_H], "${WITH_JVM_MD_H}/jvm_md.h", [Java runtime library jvm_md.h header])],
                [AC_MSG_ERROR(cannot find jvm_md.h)])

dnl Add the path to jvm_md.h to the CPPFLAGS so jvm.h can find it.
CPPFLAGS="${CPPFLAGS} -I${WITH_JVM_MD_H}"
])


dnl Check for jvm.h

AC_DEFUN([AC_CHECK_WITH_JVM_H],[
AC_MSG_CHECKING(where jvm.h is installed)
AC_ARG_WITH([jvm_h],
            [AS_HELP_STRING(--with-jvm_h=<dir>,path to jvm.h (only with --with-java-runtime-library=openjdk) [[default=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export]])],
            [WITH_JVM_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk*)
                     WITH_JVM_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export
                     ;;
                 *)
                     ;;
            esac])
AC_MSG_RESULT(${WITH_JVM_H})

AC_CHECK_HEADER([${WITH_JVM_H}/jvm.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_JVM_H], "${WITH_JVM_H}/jvm.h", [Java runtime library jvm.h header])],
                [AC_MSG_ERROR(cannot find jvm.h)])
])
