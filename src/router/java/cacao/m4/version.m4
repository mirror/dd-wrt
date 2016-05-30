dnl m4/version.m4
dnl
dnl Copyright (C) 1996-2013
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


dnl define detailed version numbers

AC_DEFUN([AC_VERSION_DETAIL],[
version="$PACKAGE_VERSION"
if test x`echo "$version" | $SED -e 's/[[0-9a-z+]]*//g'` = "x..";
then
    major=`echo "$version" | $SED -e 's/\.[[0-9a-z.+]]*$//'`
    minor=`echo "$version" | $SED -e 's/^[[0-9]]*\.//' -e 's/\.[[0-9a-z.+]]*$//'`
    micro=`echo "$version" | $SED -e 's/^[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\).*/\1/'`
    extra=`echo "$version" | $SED -e 's/^[[0-9]]*\.[[0-9]]*\.[[0-9]]*//'`
else
    major=`echo "$version" | $SED -e 's/\.[[0-9a-z.+]]*$//'`
    minor=`echo "$version" | $SED -e 's/^[[0-9]]*\.//' -e 's/[[a-z.+]]*$//'`
    micro=0
    extra=`echo "$version" | $SED -e 's/^[[0-9]]*\.[[0-9]]*//'`
fi
if test ! "x$CACAO_HGREV" = "x";
then
    extra="${extra}+r$CACAO_HGREV"
fi

AC_DEFINE_UNQUOTED(VERSION_MAJOR, $major, [major version number])
AC_DEFINE_UNQUOTED(VERSION_MINOR, $minor, [minor version number])
AC_DEFINE_UNQUOTED(VERSION_MICRO, $micro, [micro version number])
AC_DEFINE_UNQUOTED(VERSION_EXTRA, "$extra", [extra version info])
AC_DEFINE_UNQUOTED(VERSION_FULL, "$major.$minor.$micro$extra", [full version info])
])


dnl define some stuff required for -XX:+PrintConfig

AC_DEFUN([AC_VERSION_CONFIG],[
AC_DEFINE_UNQUOTED(VERSION_CONFIGURE_ARGS, "$ac_configure_args", [configure arguments])
AC_DEFINE_UNQUOTED(VERSION_CC, "$CC", [CC used])
AC_DEFINE_UNQUOTED(VERSION_CXX, "$CXX", [CXX used])
AC_DEFINE_UNQUOTED(VERSION_CFLAGS, "$AM_CFLAGS $CFLAGS", [CFLAGS used])
AC_DEFINE_UNQUOTED(VERSION_CXXFLAGS, "$AM_CXXFLAGS $CXXFLAGS", [CXXFLAGS used])
AC_DEFINE_UNQUOTED(VERSION_CPPFLAGS, "$AM_CPPFLAGS $CPPFLAGS", [CPPFLAGS used])
])
