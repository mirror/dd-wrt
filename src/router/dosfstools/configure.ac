# configure.ac for dosfstools
# Copyright (C) 2015  Andreas Bombe <aeb@debian.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([dosfstools], [4.1])
AC_SUBST([RELEASE_DATE], [2017-01-24])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects parallel-tests])

AC_ARG_ENABLE([compat-symlinks],
	[AS_HELP_STRING([--enable-compat-symlinks],
		      [install symlinks for legacy names of the tools])],
	[case "${enableval}" in
	yes) symlinks=true ;;
	no)  symlinks=false ;;
	*)   AC_MSG_ERROR([bad value ${enableval} for --enable-compat-symlinks]) ;;
	esac],
	[symlinks=false])
AM_CONDITIONAL([COMPAT_SYMLINKS], [test x$symlinks = xtrue])


AC_PROG_CC
AC_PROG_LN_S

AC_SYS_LARGEFILE

AC_CHECK_HEADERS([\
		  err.h \
		  linux/fd.h \
		  linux/hdreg.h \
		  linux/version.h \
		  sys/disk.h \
		  sys/disklabel.h \
		  sys/ioccom.h \
		  sys/mkdev.h \
		  sys/queue.h \
		  ])

AC_CHECK_HEADERS([endian.h sys/endian.h libkern/OSByteOrder.h])

AC_CHECK_DECLS([getmntent], [], [], [[#include <mntent.h>]])
AC_CHECK_DECLS([getmntinfo], [], [], [[#include <sys/mount.h>]])

AC_ARG_WITH([udev], AS_HELP_STRING([--without-udev], [build without libudev support]))
if test "x$with_udev" != "xno"; then
  PKG_CHECK_MODULES([UDEV], [libudev],
		  [AC_DEFINE([HAVE_UDEV], [1])],
		  [true])
fi

AC_SEARCH_LIBS(iconv_open, iconv)

# xxd (distributed with vim) is used in the testsuite
AC_CHECK_PROG([XXD_FOUND], [xxd], [yes])

# check if automake is 1.11 or 1.12 for using a workaround for the parallel test harness
# (we check for a minimum version of 1.11 above, so these are the only two values to check)
AM_CONDITIONAL(AUTOMAKE_TEST_COMPAT, test $am__api_version = 1.11 -o $am__api_version = 1.12)

AC_CONFIG_FILES([Makefile src/Makefile src/version.h
		 manpages/Makefile manpages/mkfs.fat.8
		 manpages/fsck.fat.8 manpages/fatlabel.8
		 tests/Makefile])
AC_OUTPUT
