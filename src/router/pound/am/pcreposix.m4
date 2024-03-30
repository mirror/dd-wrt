# SYNOPSIS
#
#   PND_PCREPOSIX
#
# DESCRIPTION
#
#   Checks whether the pcreposix library and its headers are available.
#   Prefers libpcre2 over libpcre.  The --enable-pcreposix option can
#   be used to enable, disable, or force the use of libpcre verison 1
#   (--enable-pcreposix=pcre1).  Upon return, the status_pcreposix shell
#   variable is set to indicate the result:
#
#   .  no - neither library has been found
#   .  1  - libpcre is found
#   .  2  - libpcre2 is found
#
#   On success, the HAVE_LIBPCREPOSIX m4 macro is defined to the version
#   of the library used (1 or 2).
#
#   Substitution variables PCREPOSIX_CFLAGS and PCREPOSIX_LIBS are defined
#   to compiler and loader flags needed in order to build with the version
#   of the library located.
#
# LICENSE
#
# Copyright (C) 2023-2024 Sergey Poznyakoff
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

AC_DEFUN([PND_PCREPOSIX],
[AC_ARG_ENABLE([pcreposix],
 [AS_HELP_STRING([--enable-pcreposix],[enable or disable using the pcreposix library (default: enabled if available)])],
 [status_pcreposix=${enableval}],
 [status_pcreposix=yes])

 AH_TEMPLATE([HAVE_LIBPCREPOSIX],[Define to the version of libpcreposix to use])

 AC_SUBST([PCREPOSIX_CFLAGS])
 AC_SUBST([PCREPOSIX_LIBS])
 if test "$status_pcreposix" != no; then
   AC_PATH_PROG([PCRE2_CONFIG],[pcre2-config],[])
   if test "$status_pcreposix" != pcre1 && test -n "$PCRE2_CONFIG"; then
     PCREPOSIX_CFLAGS=$($PCRE2_CONFIG --cflags-posix)
     PCREPOSIX_LIBS=$($PCRE2_CONFIG --libs-posix)
     status_pcreposix=2
     # Debian build of pcre2posix is badly broken.  For some obscure
     # reason its maintainer decided to rename reg* functions by
     # prefixing them with PCRE2, which defeats the main purpose of the
     # library.  To make matters even worse, he didn't do the same to the
     # regex_t type, which means that linking with Debian build of
     # libpcre2posix results in memory overrruns when regcomp is called
     # (pcre2posix definition of regex_t is smaller than the one in libc).
     #
     # The code below attempts to detect the deficiency and install a
     # workaround.
     saved_CFLAGS=$CFLAGS
     CFLAGS="$CFLAGS $PCREPOSIX_CFLAGS"
     saved_LIBS=$LIBS
     LIBS="$LIBS $PCREPOSIX_LIBS"
     AC_LINK_IFELSE(
         [AC_LANG_PROGRAM([], [PCRE2regcomp()])],
	 [AC_DEFINE([regcomp],[PCRE2regcomp],
	            [Compensate for Debian deficiency])
	  AC_DEFINE([regexec],[PCRE2regexec],
	            [Compensate for Debian deficiency])
	  AC_DEFINE([regerror],[PCRE2regerror],
	            [Compensate for Debian deficiency])
	  AC_DEFINE([regfree],[PCRE2regfree],
	            [Compensate for Debian deficiency])])
     LIBS=$saved_LIBS
     CFLAGS=$saved_CFLAGS
   else
     AC_CHECK_HEADERS([pcreposix.h pcre/pcreposix.h])
     AC_CHECK_LIB([pcre],[pcre_compile],
       [PCREPOSIX_LIBS=-lpcre
        AC_CHECK_LIB([pcreposix],[regcomp],
	   [PCREPOSIX_LIBS="$PCREPOSIX_LIBS -lpcreposix"
	    status_pcreposix=1],
	   [status_pcreposix=no],
	   [$PCREPOSIX_LIBS])],
       [status_pcreposix=no])
   fi       

   case "$status_pcreposix" in
   1|2)  AC_DEFINE_UNQUOTED([HAVE_LIBPCREPOSIX],[$status_pcreposix])
   esac
 fi
])
