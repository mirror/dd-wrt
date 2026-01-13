# SYNOPSIS
#
#   PND_PCRE
#
# DESCRIPTION
#
#   Checks whether the pcre library and its headers are available.
#   Prefers libpcre2 over libpcre.  The --enable-pcre option can
#   be used to enable, disable, or force the use of libpcre version 1
#   (--enable-pcre=1).  Upon return, the status_pcre shell variable is
#   set to indicate the result:
#
#   .  no - neither library has been found
#   .  1  - libpcre is found
#   .  2  - libpcre2 is found
#
#   On success, the HAVE_LIBPCRE m4 macro is defined to the version
#   of the library used (1 or 2).
#
#   Substitution variables PCRE_CFLAGS and PCRE_LIBS are defined
#   to compiler and loader flags needed in order to build with the version
#   of the library located.
#
# LICENSE
#
# Copyright (C) 2023-2025 Sergey Poznyakoff
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_DEFUN([PND_PCRE],
[AC_ARG_ENABLE([pcre],
 [AS_HELP_STRING([--enable-pcre],[enable or disable using the pcre library (default: enabled if available)])],
 [status_pcre=${enableval}],
 [status_pcre=yes])

 AH_TEMPLATE([HAVE_LIBPCRE],[Define to the version of libpcreposix to use])

 AC_SUBST([PCRE_CFLAGS])
 AC_SUBST([PCRE_LIBS])
 if test "$status_pcre" != no; then
   AC_PATH_PROG([PCRE2_CONFIG],[pcre2-config],[])
   if test "$status_pcre" != 1 && test -n "$PCRE2_CONFIG"; then
     PCRE_CFLAGS=$($PCRE2_CONFIG --cflags)
     PCRE_LIBS=$($PCRE2_CONFIG --libs8)
     status_pcre=2
   else
     AC_CHECK_HEADERS([pcre.h pcre/pcre.h])
     AC_CHECK_LIB([pcre],[pcre_compile],
       [PCRE_LIBS=-lpcre
	status_pcre=1],
       [status_pcre=no])
   fi

   case "$status_pcre" in
   1|2)  AC_DEFINE_UNQUOTED([HAVE_LIBPCRE],[$status_pcre])
   esac
 fi
])
