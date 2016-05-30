dnl m4/hpi.m4
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


dnl where hpi_md.h is installed

AC_DEFUN([AC_CHECK_WITH_HPI_MD_H],[
AC_MSG_CHECKING(where hpi_md.h is installed)
AC_ARG_WITH([hpi_md_h],
            [AS_HELP_STRING(--with-hpi_md_h=<dir>,path to hpi_md.h (only with --with-java-runtime-library=openjdk) [[default=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/hpi/export]])],
            [WITH_HPI_MD_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk)
                     WITH_HPI_MD_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/hpi/export
                     ;;
                 *)
                     ;;
            esac])
AC_MSG_RESULT(${WITH_HPI_MD_H})

dnl We use CPPFLAGS so hpi.h can find hpi_md.h
CPPFLAGS="${CPPFLAGS} -I${WITH_HPI_MD_H}"

AC_CHECK_HEADER([${WITH_HPI_MD_H}/hpi_md.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_HPI_MD_H], "${WITH_HPI_MD_H}/hpi_md.h", [Java core library hpi_md.h header])],
                [AC_MSG_ERROR(cannot find hpi_md.h)])
])


dnl where hpi.h is installed

AC_DEFUN([AC_CHECK_WITH_HPI_H],[
AC_MSG_CHECKING(where hpi.h is installed)
AC_ARG_WITH([hpi_h],
            [AS_HELP_STRING(--with-hpi_h=<dir>,path to hpi.h (only with --with-java-runtime-library=openjdk) [[default=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/hpi/export]])],
            [WITH_HPI_H=${withval}],
            [WITH_HPI_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/hpi/export])
AC_MSG_RESULT(${WITH_HPI_H})

dnl We use CPPFLAGS so hpi.h can find hpi_md.h
CPPFLAGS="${CPPFLAGS} -I${WITH_HPI_H}"

AC_CHECK_HEADER([${WITH_HPI_H}/hpi.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_HPI_H], "${WITH_HPI_H}/hpi.h", [Java core library hpi.h header])],
                [AC_MSG_ERROR(cannot find hpi.h)],
                [#include INCLUDE_HPI_MD_H])
])
