# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([gettext-dummy], [1.0.1], [bugs@pengutronix.de])
AC_CONFIG_SRCDIR([libintl.h])
AC_CONFIG_HEADER([config.h])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_MAINTAINER_MODE

CFLAGS="${CFLAGS} -Wall -Wsign-compare -Wfloat-equal -Wformat-security"

#
# libtool library versioning stuff
#
# Library code modified:	    REVISION++
# Interfaces changed/added/removed: CURRENT++	REVISION=0
# Interfaces added:		    AGE++
# Interfaces removed:		    AGE=0
LT_CURRENT=1
LT_REVISION=1
LT_AGE=0
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2])

#
# Checks for libraries.
#

#
# Checks for header files.
#

#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST

#
# Debugging
#
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug], [enable debugging @<:@default=no@:>@]),
	[case "$enableval" in
	y | yes) CONFIG_DEBUG=yes ;;
        *) CONFIG_DEBUG=no ;;
    esac],
    [CONFIG_DEBUG=no])
AC_MSG_RESULT([${CONFIG_DEBUG}])
if test "${CONFIG_DEBUG}" = "yes"; then
    CFLAGS="${CFLAGS} -g -O1"
    AC_DEFINE(DEBUG, 1, [debugging])
else
    CFLAGS="${CFLAGS} -Werror -O2"
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

