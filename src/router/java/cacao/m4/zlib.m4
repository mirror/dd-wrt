dnl m4/zlib.m4
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


dnl check if zlib should be used

AC_DEFUN([AC_CHECK_ENABLE_ZLIB],[
AC_MSG_CHECKING(whether ZIP/JAR archives should be supported)
AC_ARG_ENABLE([zlib],
              [AS_HELP_STRING(--disable-zlib,disable ZIP/JAR archive support (needs zlib) [[default=enabled]])],
              [case "${enableval}" in
                  no) ENABLE_ZLIB=no;;
                  *) ENABLE_ZLIB=yes;;
               esac],
              [ENABLE_ZLIB=yes])
AC_MSG_RESULT(${ENABLE_ZLIB})

if test x"${ENABLE_ZLIB}" = "xyes"; then
    AC_CHECK_HEADERS([zconf.h],, [AC_MSG_ERROR(cannot find zconf.h)])
    AC_CHECK_HEADERS([zlib.h],, [AC_MSG_ERROR(cannot find zlib.h)])
    AC_CHECK_LIB(z, inflate,, [AC_MSG_ERROR(cannot find libz)])
    AC_DEFINE([ENABLE_ZLIB], 1, [use zlib])
fi
AM_CONDITIONAL([ENABLE_ZLIB], test x"${ENABLE_ZLIB}" = "xyes")
])
