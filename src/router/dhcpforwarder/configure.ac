dnl Copyright (C) 2002, 2003, 2004, 2008, 2010, 2012, 2014
dnl               Enrico Scholz <enrico.scholz@ensc.de>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 3 of the License.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program. If not, see http://www.gnu.org/licenses/.

# Process this file with 'autoreconf -i' to produce a configure script.
AC_INIT(dhcp-forwarder, 0.11, [enrico.scholz@ensc.de], [dhcp-forwarder])
AC_CONFIG_SRCDIR([THANKS])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.6.0 gnits dist-xz subdir-objects])

AM_MAINTAINER_MODE
AM_SILENT_RULES([no])
ENSC_CHANGELOG([master],[git])

# Default settings

enable_logging_default=no


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_LN_S

AC_PATH_PROGS(SPLINT, splint)

ENSC_STANDARD_TOOLS
ENSC_RELEASE(IS_RELEASE)

ENSC_ENABLE_DIETLIBC(USE_DIETLIBC)
ENSC_INITRDDIR(initrddir)

AC_DEFINE(_GNU_SOURCE, 1, [Uses GNU features])

ENSC_CHECK_CC_FLAG(${RELEASE_CFLAGS})
ENSC_CHECK_CC_FLAG([-Wall -W -Wshadow])

if test x"${ensc_use_dietlibc}" != xyes; then
	dnl dietlibc <= 0.17 does not compile with -std=c99... :(
	ENSC_CHECK_CC_FLAG([-std=c99 -pedantic])
elif which $CC gcc &>/dev/null; then
	dnl Fix cross-compiler detection of dietlibc since it is
	dnl broken when using names like i386-redhat-linux-gcc
	if test x"$CC" != xgcc; then
		i1=`ls -i $(which $CC) | { read a b; echo $a; }`
		i2=`ls -i $(which gcc) | { read a b; echo $a; }`

		AC_MSG_CHECKING([whether to use gcc instead of $CC])
		if test $i1 = $i2; then
			CC=gcc
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
		fi
	fi
fi

ENSC_MODERN_COMPILER_CHECK

AC_MSG_CHECKING(for systemd unitdir)
AC_ARG_WITH(systemd-unitdir,
	    [AC_HELP_STRING([--with-systemd-unitdir=<dir>],
                            [The systemd unit directory (default:
                             <prefix>/lib/systemd/system)])],
	    [],[with_systemd_unitdir=\${prefix}/lib/systemd/system])
AC_MSG_RESULT(${with_systemd_unitdir})
AC_SUBST(SYSTEMD_UNITDIR,[${with_systemd_unitdir}])

AC_MSG_CHECKING(whether to enable logging)
AC_ARG_ENABLE(logging,
	      [AC_HELP_STRING([--enable-logging],
			      [Enables logging of received
			       packages. Will increase code-size and
			       adds potential attack-points (disabled
			       by default)])],
	      [case "${enableval}" in
		 yes|no)  enable_logging="${enableval}";;
		 *)       AC_MSG_ERROR([bad value ${enableval} for '--enable-logging']);;
	       esac],
	      [enable_logging=${enable_logging_default}])
AC_MSG_RESULT(${enable_logging})

if test x"${enable_logging}" = xyes; then
	AC_DEFINE(WITH_LOGGING, 1, [Enables logging if set])
	AM_CONDITIONAL(WITH_LOGGING, true)
else
	AM_CONDITIONAL(WITH_LOGGING, false)
fi

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h netinet/in.h stdint.h])
AC_CHECK_HEADERS([stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h])
AC_CHECK_HEADERS([unistd.h stdbool.h])

# Checks for typedefs, structures, and compiler characteristics.
ENSC_DIET_CHECK_IN_ADDR_T
ENSC_DIET_TYPE_CHECK(DIET_HAS_STRUCT_ETHER_HEADER, [struct ether_header],
		     [#include <netinet/ether.h>])

old_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Werror -Wno-unused -Wno-uninitialized"
AC_C_CONST
AC_C_INLINE
CFLAGS="$old_CFLAGS"

AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_STRUCT_TM


# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_PROG_GCC_TRADITIONAL

ENSC_DIET_CHECK_ETHER


AH_TOP([
#ifndef H_DHCP_FORWARDER_TOP_CONFIG_H
#define H_DHCP_FORWARDER_TOP_CONFIG_H
/*@-namechecks@*/
])

AH_BOTTOM([
/*@=namechecks@*/
#endif	/* H_DHCP_FORWARDER_TOP_CONFIG_H */
])

AC_CONFIG_FILES([Makefile test/Makefile dhcp-forwarder.spec])
AC_OUTPUT
